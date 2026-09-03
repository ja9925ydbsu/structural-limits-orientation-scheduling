#!/usr/bin/env python3
"""Revision-6 model-exact weight-one transition and periodic transfer-rate study.

Uses the validated matched-control implementation as the normative source for
key derivation, matrix schedules, routing, and AES S-box DDT transitions.
Outputs per-context finite-round survival probabilities for 256 deterministic matrix contexts,
summary statistics with confidence intervals, and asymptotic growth rates of the
actual 16-round periodic 128-state transfer operator.
"""
from __future__ import annotations
import argparse, csv, json, math, time
from pathlib import Path
from statistics import mean, stdev
import numpy as np
import matched_orientation_schedule_experiment as base

VARIANTS = ("static", "position_only", "rotor", "round_only")
ROUNDS = (4, 8, 12, 16)


def make_edge_arrays(transitions):
    out=[]
    for tr in transitions:
        src=[]; dst=[]; prob=[]
        for s, edges in enumerate(tr):
            for d,c in edges:
                src.append(s); dst.append(d); prob.append(c/256.0)
        out.append((np.asarray(src,dtype=np.int16), np.asarray(dst,dtype=np.int16), np.asarray(prob,dtype=np.float64)))
    return out


def apply_T_col(edge, v):
    src,dst,p=edge
    # (T v)[src] = sum_dst p(src,dst) v[dst]
    return np.bincount(src, weights=p*v[dst], minlength=128).astype(np.float64, copy=False)


def survival_by_round(edges, report_rounds=ROUNDS):
    # q_r(s) = probability of surviving in W1 from boundary s through rounds r..R-1.
    # To get every prefix R independently, reverse only the first R matrices.
    vals={}
    starts={}
    for R in report_rounds:
        q=np.ones(128,dtype=np.float64)
        for r in range(R-1,-1,-1):
            q=apply_T_col(edges[r],q)
        i=int(np.argmax(q)); p=float(q[i])
        vals[R]=p; starts[R]=i
    return vals, starts


def best_single_by_round(transitions, report_rounds=ROUNDS):
    v=[1.0]*128
    out={}
    for r,tr in enumerate(transitions):
        new=[0.0]*128
        for src,edges in enumerate(tr):
            vs=v[src]
            if vs==0.0: continue
            for dst,count in edges:
                cand=vs*(count/256.0)
                if cand>new[dst]: new[dst]=cand
        v=new
        rr=r+1
        if rr in report_rounds: out[rr]=max(v)
    return out

def finite_slope(vals):
    # least-squares decay in -log2 probability versus round count, matching manuscript convention
    xs=np.asarray(ROUNDS,dtype=float)
    ys=np.asarray([-math.log2(vals[r]) for r in ROUNDS],dtype=float)
    return float(np.polyfit(xs,ys,1)[0])


def periodic_growth_rate(edges, tol=1e-13, max_iter=250):
    # P = T0 T1 ... T15 acting on a column survival vector.  Power iteration
    # applies T15 first and T0 last. The per-cycle Perron factor lambda gives
    # rate = -log2(lambda)/16.
    v=np.ones(128,dtype=np.float64)
    v/=v.sum()
    last=None
    lam=0.0
    for it in range(1,max_iter+1):
        w=v
        for r in range(15,-1,-1):
            w=apply_T_col(edges[r],w)
        lam=float(w.sum())
        if lam <= 0.0:
            return float("inf"), 0.0, it
        w/=lam
        if last is not None and abs(math.log(lam)-math.log(last)) < tol:
            v=w; break
        last=lam; v=w
    return -math.log2(lam)/16.0, lam, it


def ci95(values):
    n=len(values); m=mean(values)
    if n < 2: return m, float('nan'), float('nan'), float('nan')
    sd=stdev(values); half=1.96*sd/math.sqrt(n)
    return m,sd,m-half,m+half


def write_csv(path, rows):
    if not rows: return
    with open(path,'w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--keys',type=int,default=256)
    ap.add_argument('--transfer-keys',type=int,default=256,
                    help='number of leading deterministic keys for periodic transfer-rate calculation')
    ap.add_argument('--out',type=Path,default=Path('weight1_transfer_256_results'))
    args=ap.parse_args(); args.out.mkdir(parents=True,exist_ok=True)
    t0=time.time(); ddt=base.aes_ddt()
    rows=[]; transfer_rows=[]; one_step_rows=[]
    for ki in range(args.keys):
        ctx=base.KeyContext.build(base.deterministic_master_key(ki))
        for variant in VARIANTS:
            transitions=base.build_weight1_transitions(ctx,base.SCHEDULES[variant],ddt)
            edges=make_edge_arrays(transitions)
            vals,starts=survival_by_round(edges)
            singles=best_single_by_round(transitions)
            slope=finite_slope(vals)
            # Mean one-step retention over all 128 boundary states and all 16 phases.
            row_sums=[]; distinct_inputs=set()
            for r,tr in enumerate(transitions):
                for src, ed in enumerate(tr):
                    row_sums.append(sum(c/256.0 for _,c in ed))
                k=base.K_VALUES[r]
                for bit_position in range(128):
                    rotated_position=(bit_position-k)%128
                    byte_index,input_bit_in_byte=divmod(rotated_position,8)
                    input_value=1 << (7-input_bit_in_byte)
                    table=ctx.tables[byte_index][base.SCHEDULES[variant](r,byte_index)%4]
                    distinct_inputs.add(int(table[input_value]))
            qmean=float(np.mean(row_sums))
            one_step_rows.append({'context_index':ki,'variant':variant,'mean_one_step_retention_probability':f'{qmean:.17g}',
                                  'neglog2_mean_one_step_retention':f'{-math.log2(qmean):.12f}',
                                  'distinct_sbox_input_differences_over_16_rounds':len(distinct_inputs)})
            for R in ROUNDS:
                rows.append({
                    'context_index':ki,'variant':variant,'rounds':R,
                    'max_weight1_class_probability':f'{vals[R]:.17g}',
                    'max_weight1_class_log2_probability':f'{math.log2(vals[R]):.12f}',
                    'max_weight1_class_start_bit':starts[R],
                    'max_single_trail_probability':f'{singles[R]:.17g}',
                    'max_single_trail_log2_probability':f'{math.log2(singles[R]):.12f}',
                    'class_minus_single_gap_bits':f'{math.log2(vals[R])-math.log2(singles[R]):.12f}',
                    'finite_4_16_decay_bits_per_round':f'{slope:.12f}',
                })
            if ki < args.transfer_keys:
                rate,lam,it=periodic_growth_rate(edges)
                transfer_rows.append({
                    'context_index':ki,'variant':variant,
                    'period_rounds':16,'spectral_decay_bits_per_round':f'{rate:.12f}',
                    'period_perron_factor':f'{lam:.17g}','iterations':it,
                })
        if (ki+1)%16==0:
            print(f'completed {ki+1}/{args.keys} keys',flush=True)
    write_csv(args.out/'model_weight1_256_by_context.csv',rows)
    write_csv(args.out/'periodic_transfer_rates_by_key.csv',transfer_rows)
    write_csv(args.out/'one_step_retention_by_key.csv',one_step_rows)

    summary=[]
    for variant in VARIANTS:
        vrows=[r for r in rows if r['variant']==variant]
        slopes=[]
        seen=set()
        for r in vrows:
            k=int(r['context_index'])
            if k not in seen:
                slopes.append(float(r['finite_4_16_decay_bits_per_round'])); seen.add(k)
        slope_m,slope_sd,slope_lo,slope_hi=ci95(slopes)
        trates=[float(r['spectral_decay_bits_per_round']) for r in transfer_rows if r['variant']==variant]
        tm,tsd,tlo,thi=ci95(trates)
        qmeans=[float(r['mean_one_step_retention_probability']) for r in one_step_rows if r['variant']==variant]
        qm,qsd,qlo,qhi=ci95(qmeans)
        uniq=[float(r['distinct_sbox_input_differences_over_16_rounds']) for r in one_step_rows if r['variant']==variant]
        um,usd,ulo,uhi=ci95(uniq)
        for R in ROUNDS:
            valslog=[float(r['max_weight1_class_log2_probability']) for r in vrows if int(r['rounds'])==R]
            m,sd,lo,hi=ci95(valslog)
            summary.append({
                'variant':variant,'rounds':R,'n_keys':len(valslog),
                'mean_log2_class_probability':f'{m:.6f}','sd_log2_class_probability':f'{sd:.6f}',
                'ci95_low_log2':f'{lo:.6f}','ci95_high_log2':f'{hi:.6f}',
                'mean_finite_4_16_decay_bits_per_round':f'{slope_m:.6f}',
                'sd_finite_decay':f'{slope_sd:.6f}','finite_decay_ci95_low':f'{slope_lo:.6f}','finite_decay_ci95_high':f'{slope_hi:.6f}',
                'transfer_n_keys':len(trates),'mean_transfer_decay_bits_per_round':f'{tm:.6f}',
                'sd_transfer_decay':f'{tsd:.6f}','transfer_decay_ci95_low':f'{tlo:.6f}','transfer_decay_ci95_high':f'{thi:.6f}',
                'mean_one_step_retention_probability':f'{qm:.9f}','neglog2_mean_one_step_retention':f'{-math.log2(qm):.6f}',
                'mean_distinct_sbox_input_differences':f'{um:.3f}',
            })
    write_csv(args.out/'weight1_256_summary.csv',summary)

    p=2048/(255*256)
    null_rate=-math.log2(p)
    meta={
        'keys':args.keys,'transfer_keys':args.transfer_keys,'variants':list(VARIANTS),
        'rounds':list(ROUNDS),'mean_field_probability':p,'mean_field_decay_bits_per_round':null_rate,
        'identity':'sum_{alpha!=0} sum_{wt(beta)=1} DDT(alpha,beta) = 8*256 = 2048; divide by 255*256',
        'key_derivation':'SHA256(b"HESPN-MATCHED-CONTROL-KEY" || uint32_be(index))',
        'model_assumption':'Exact within the Markov-cipher DDT transition model; not a proof for a fixed SHA-256-derived keyed permutation.',
        'runtime_seconds':time.time()-t0,
    }
    (args.out/'analysis_metadata.json').write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(meta,indent=2))
    return 0

if __name__=='__main__': raise SystemExit(main())
