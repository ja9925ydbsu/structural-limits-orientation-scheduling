#!/usr/bin/env python3
from __future__ import annotations
import csv, hashlib, json, math, statistics, time
from collections import Counter, defaultdict
from pathlib import Path
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import connected_components

import matched_orientation_schedule_experiment as base
import weight1_transfer_256 as wt

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent.parent
RESULTS=ROOT/'results'
OUT=RESULTS/'reviewer_revision'
OUT.mkdir(parents=True,exist_ok=True)
VARIANTS=wt.VARIANTS
PRIMARY_LABEL=b'HESPN-MATCHED-CONTROL-KEY'
REPL_LABEL=b'HESPN-MATCHED-CONTROL-KEY-REPLICATION'

def key_from_label(label:bytes,i:int)->bytes:
    return hashlib.sha256(label+i.to_bytes(4,'big')).digest()

def quantiles(xs, qs=(0, .05,.25,.5,.75,.95,1)):
    a=np.asarray(xs,dtype=float)
    return {str(q):float(np.quantile(a,q)) for q in qs}

def dense_T(transitions_round):
    T=np.zeros((128,128),dtype=np.float64)
    for s,edges in enumerate(transitions_round):
        for d,c in edges:
            T[s,d]=c/256.0
    return T

def dense_period(transitions):
    P=np.eye(128,dtype=np.float64)
    for tr in transitions:
        P=P@dense_T(tr)
    return P

def all_start_survival(edges,R=16):
    q=np.ones(128,dtype=np.float64)
    for r in range(R-1,-1,-1):
        q=wt.apply_T_col(edges[r],q)
    return q

def panel_context_summary(label:bytes, n:int, need_starts=False, need_acceptance=False):
    ddt=base.aes_ddt()
    rows=[]; start_rows=[]; attempts=[]
    t0=time.time()
    for ki in range(n):
        ctx=base.KeyContext.build(key_from_label(label,ki))
        if need_acceptance:
            attempts.append({'context':ki,'total_candidates':sum(ctx.seed_attempts),'attempts':ctx.seed_attempts})
        for variant in VARIANTS:
            tr=base.build_weight1_transitions(ctx,base.SCHEDULES[variant],ddt)
            edges=wt.make_edge_arrays(tr)
            rate,lam,it=wt.periodic_growth_rate(edges)
            q=all_start_survival(edges,16)
            rows.append({'context':ki,'variant':variant,'periodic_rate':rate,'lambda':lam,'iterations':it,
                         'oracle_p16':float(q.max()),'oracle_log2_p16':float(np.log2(q.max())),
                         'oracle_start':int(np.argmax(q)),'fixed0_p16':float(q[0]),
                         'fixed0_log2_p16':float(np.log2(q[0])) if q[0]>0 else float('-inf')})
            if need_starts:
                for s,p in enumerate(q):
                    start_rows.append({'context':ki,'variant':variant,'start_bit':s,'p16':float(p),'log2_p16':float(np.log2(p)) if p>0 else float('-inf')})
        if (ki+1)%32==0:
            print(label.decode(),ki+1,'/',n,'elapsed',round(time.time()-t0,1),flush=True)
    return rows,start_rows,attempts

def summarize_panel(rows,n_values):
    out=[]
    by={(r['context'],r['variant']):r for r in rows}
    for n in n_values:
        for v2 in ('rotor','round_only'):
            diffs=np.array([by[(i,v2)]['periodic_rate']-by[(i,'static')]['periodic_rate'] for i in range(n)])
            out.append({'n':n,'contrast':f'{v2}-static','mean':float(diffs.mean()),'sd':float(diffs.std(ddof=1)),
                        'median':float(np.median(diffs)),'quantiles':quantiles(diffs),'positive':int((diffs>0).sum()),
                        'negative':int((diffs<0).sum()),'zero':int((diffs==0).sum())})
    return out

def starting_summary(rows,start_rows):
    out={}
    byvar=defaultdict(list)
    for r in start_rows: byvar[r['variant']].append(r)
    rowmap={(r['context'],r['variant']):r for r in rows}
    for v in VARIANTS:
        vals=np.array([r['log2_p16'] for r in byvar[v]])
        arg=Counter(rowmap[(i,v)]['oracle_start'] for i in range(256))
        out[v]={
            'all_context_start_log2_quantiles':quantiles(vals),
            'modal_argmax_bit':arg.most_common(1)[0][0],
            'modal_argmax_count':arg.most_common(1)[0][1],
            'distinct_argmax_bits':len(arg),
            'oracle_mean_log2':float(np.mean([rowmap[(i,v)]['oracle_log2_p16'] for i in range(256)])),
            'oracle_log2_arithmetic_mean_p':float(np.log2(np.mean([rowmap[(i,v)]['oracle_p16'] for i in range(256)]))),
            'fixed0_mean_log2':float(np.mean([rowmap[(i,v)]['fixed0_log2_p16'] for i in range(256)])),
            'fixed0_log2_arithmetic_mean_p':float(np.log2(np.mean([rowmap[(i,v)]['fixed0_p16'] for i in range(256)]))),
        }
    for sel,key in [('oracle','oracle_log2_p16'),('fixed0','fixed0_log2_p16')]:
        diffs=np.array([rowmap[(i,'rotor')][key]-rowmap[(i,'static')][key] for i in range(256)])
        # log2 probability contrast; more negative means rotor lower survival. Report static-minus-rotor decay benefit positive.
        out[f'rotor_static_{sel}_log2_probability_difference']={'mean':float(diffs.mean()),'median':float(np.median(diffs)),'quantiles':quantiles(diffs)}
    return out

def acceptance_summary(attempts):
    totals=np.array([x['total_candidates'] for x in attempts],dtype=float)
    indiv=np.array([a for x in attempts for a in x['attempts']],dtype=float)
    return {
        'contexts':len(attempts),'accepted_seeds':len(attempts)*16,
        'total_candidates_all_contexts':int(totals.sum()),
        'overall_acceptance_fraction':float((len(attempts)*16)/totals.sum()),
        'candidates_per_context_mean':float(totals.mean()),'median':float(np.median(totals)),'min':int(totals.min()),'max':int(totals.max()),
        'candidates_per_context_quantiles':quantiles(totals),
        'attempts_per_accepted_seed_mean':float(indiv.mean()),'median':float(np.median(indiv)),'max':int(indiv.max()),'quantiles':quantiles(indiv)
    }

def numerical_validation(n=32):
    ddt=base.aes_ddt(); rec=[]
    for i in range(n):
        ctx=base.KeyContext.build(key_from_label(PRIMARY_LABEL,i))
        for v in VARIANTS:
            tr=base.build_weight1_transitions(ctx,base.SCHEDULES[v],ddt)
            edges=wt.make_edge_arrays(tr)
            rate_pi,lam,it=wt.periodic_growth_rate(edges,tol=1e-13,max_iter=500)
            P=dense_period(tr)
            vals=np.linalg.eigvals(P)
            rho=float(np.max(np.abs(vals)))
            rate_eig=-math.log2(rho)/16
            # scaled long-horizon dynamic iteration of explicit P
            x=np.ones(128,dtype=np.float64); x/=x.sum(); last=None
            for _ in range(100):
                y=P@x
                scale=float(y.sum())
                x=y/scale
                last=scale
            rate_dp=-math.log2(last)/16
            ncomp,_=connected_components(csr_matrix(P>0),directed=True,connection='strong')
            rec.append({'context':i,'variant':v,'power_rate':rate_pi,'eig_rate':rate_eig,'dp100_rate':rate_dp,
                        'abs_power_eig_diff':abs(rate_pi-rate_eig),'abs_power_dp_diff':abs(rate_pi-rate_dp),
                        'power_iterations':it,'strong_components':int(ncomp),'rho':rho})
    return {
        'n_contexts':n,'n_schedule_contexts':len(rec),
        'max_abs_power_eig_rate_diff':max(r['abs_power_eig_diff'] for r in rec),
        'max_abs_power_dp100_rate_diff':max(r['abs_power_dp_diff'] for r in rec),
        'max_power_iterations':max(r['power_iterations'] for r in rec),
        'reducible_count':sum(r['strong_components']>1 for r in rec),
        'max_strong_components':max(r['strong_components'] for r in rec),
        'records':rec,
    }

def round_step_np(state,ctx,r,schedule):
    # Copy of validated vectorized round logic, exposed one round at a time for calibration.
    n=state.shape[0]; state=np.ascontiguousarray(state,dtype=np.uint8).reshape(-1,16)
    tables=getattr(ctx,'_np_tables',None)
    if tables is None:
        tables=np.empty((16,4,256),dtype=np.uint8)
        for j in range(16):
            for k in range(4): tables[j,k,:]=np.frombuffer(ctx.tables[j][k],dtype=np.uint8)
        ctx._np_tables=tables
        ctx._np_round_keys=np.frombuffer(b''.join(ctx.round_keys),dtype=np.uint8).reshape(16,16)
        ctx._np_sbox=np.asarray(base.AES_SBOX,dtype=np.uint8)
    k=base.K_VALUES[r]
    words=state.view(dtype='>u8').reshape(n,2)
    hi=words[:,0].astype(np.uint64); lo=words[:,1].astype(np.uint64)
    new_hi=(hi<<np.uint64(k)) | (lo>>np.uint64(64-k))
    new_lo=(lo<<np.uint64(k)) | (hi>>np.uint64(64-k))
    rw=np.empty((n,2),dtype='>u8'); rw[:,0]=new_hi; rw[:,1]=new_lo
    state=rw.view(np.uint8).reshape(n,16)
    state=np.bitwise_xor(state,ctx._np_round_keys[r])
    mixed=np.empty_like(state)
    for j in range(16): mixed[:,j]=tables[j,schedule(r,j)%4,state[:,j]]
    sub=ctx._np_sbox[mixed]
    dest=np.asarray([base.routing_pi(r,j) for j in range(16)],dtype=int)
    routed=np.empty_like(sub); routed[:,dest]=sub
    return routed

def calibration_r2(contexts=(0,1,2,3), starts=(0,17,34,51,68,85,102,119), n_pairs=131072):
    ddt=base.aes_ddt(); rng=np.random.default_rng(20260911); records=[]
    for ci in contexts:
        ctx=base.KeyContext.build(key_from_label(PRIMARY_LABEL,ci))
        # independent deterministic plaintext panel per context/start, reused across schedules
        for s in starts:
            seed=20260911 + ci*1000+s
            rr=np.random.default_rng(seed)
            a=rr.integers(0,256,size=(n_pairs,16),dtype=np.uint8)
            b=a.copy(); byte=s//8; bit=s%8; b[:,byte]^=np.uint8(1<<(7-bit))
            for v in VARIANTS:
                schedule=base.SCHEDULES[v]
                tr=base.build_weight1_transitions(ctx,schedule,ddt); edges=wt.make_edge_arrays(tr)
                q=all_start_survival(edges,2); pred=float(q[s])
                sa=a.copy(); sb=b.copy(); alive=np.ones(n_pairs,dtype=bool)
                for r in range(2):
                    sa=round_step_np(sa,ctx,r,schedule); sb=round_step_np(sb,ctx,r,schedule)
                    diff=np.bitwise_xor(sa,sb)
                    hw=base.popcount_rows(diff)
                    alive &= (np.asarray(hw)==1)
                hits=int(alive.sum()); obs=hits/n_pairs
                se=math.sqrt(obs*(1-obs)/n_pairs) if hits else math.sqrt(max(pred,1e-300)/n_pairs)
                records.append({'context':ci,'start_bit':s,'variant':v,'pairs':n_pairs,'hits':hits,'observed':obs,'predicted':pred,
                                'obs_minus_pred':obs-pred,'z_approx':(obs-pred)/se if se>0 else None,
                                'log2_observed':math.log2(obs) if obs>0 else None,'log2_predicted':math.log2(pred) if pred>0 else None,
                                'bits_deviation':(-math.log2(obs)+math.log2(pred)) if obs>0 and pred>0 else None})
            print('calib context',ci,'start',s,flush=True)
    # aggregate by schedule with expected counts summed cellwise
    agg={}
    for v in VARIANTS:
        rv=[r for r in records if r['variant']==v]
        hits=sum(r['hits'] for r in rv); pairs=sum(r['pairs'] for r in rv); exp=sum(r['predicted']*r['pairs'] for r in rv)
        obs=hits/pairs; pred=exp/pairs
        agg[v]={'cells':len(rv),'pairs':pairs,'hits':hits,'observed':obs,'predicted':pred,
                'bits_deviation':(-math.log2(obs)+math.log2(pred)) if obs>0 else None}
    return {'n_pairs_per_cell':n_pairs,'contexts':list(contexts),'starts':list(starts),'records':records,'aggregate':agg}

def round_equivalent_decomposition():
    # Values from validated 256-context summaries.
    e_static=78.211; e_rotor=78.515; bench=121.0
    g_static=4.97395; g_rotor=4.98276
    rem_s=(bench-e_static)/g_static; rem_r=(bench-e_rotor)/g_rotor
    # Decompose exact difference into level at common rotor rate plus rate at static level.
    level=(e_rotor-e_static)/g_rotor
    rate=(bench-e_static)*(1/g_static-1/g_rotor)
    interaction=(rem_s-rem_r)-level-rate
    return {'static_remaining_rounds':rem_s,'rotor_remaining_rounds':rem_r,'advantage':rem_s-rem_r,
            'level_component_at_rotor_rate':level,'rate_component_at_static_level':rate,'interaction_residual':interaction}

def main():
    # Generate full primary 512 panel; this also yields 256-start distributions and acceptance summary for first 256.
    primary_rows, primary_starts, primary_attempts = panel_context_summary(PRIMARY_LABEL,512,need_starts=True,need_acceptance=True)
    # We only retain starting-state rows and acceptance for first 256 contexts in summary.
    primary_starts_256=[r for r in primary_starts if r['context']<256]
    attempts_256=primary_attempts[:256]
    # independent replication panel, 256 contexts
    repl_rows,_,_=panel_context_summary(REPL_LABEL,256,need_starts=False,need_acceptance=False)
    summary={
        'primary_panel_prefix_sensitivity':summarize_panel(primary_rows,[64,128,256,512]),
        'replication_panel':summarize_panel(repl_rows,[256]),
        'starting_state_summary':starting_summary([r for r in primary_rows if r['context']<256],primary_starts_256),
        'acceptance_summary':acceptance_summary(attempts_256),
        'numerical_validation':numerical_validation(32),
        'round_equivalent_decomposition':round_equivalent_decomposition(),
    }
    # calibration separately (can be expensive)
    summary['fixed_key_r2_calibration']=calibration_r2()
    (OUT/'reviewer_revision_summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    # write compact CSVs
    with open(OUT/'panel_prefix_sensitivity.csv','w',newline='') as f:
        w=csv.writer(f); w.writerow(['panel','n','contrast','mean','sd','median','q05','q25','q75','q95','positive'])
        for panel,data in [('primary',summary['primary_panel_prefix_sensitivity']),('replication',summary['replication_panel'])]:
            for r in data:
                q=r['quantiles']; w.writerow([panel,r['n'],r['contrast'],r['mean'],r['sd'],r['median'],q['0.05'],q['0.25'],q['0.75'],q['0.95'],r['positive']])
    with open(OUT/'fixed_key_r2_calibration.csv','w',newline='') as f:
        rec=summary['fixed_key_r2_calibration']['records']; w=csv.DictWriter(f,fieldnames=rec[0].keys()); w.writeheader(); w.writerows(rec)
    print(json.dumps({k:v for k,v in summary.items() if k!='fixed_key_r2_calibration'},indent=2))
    print('calibration aggregate',json.dumps(summary['fixed_key_r2_calibration']['aggregate'],indent=2))

if __name__=='__main__': main()
