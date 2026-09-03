#!/usr/bin/env python3
"""Revision 8 audit for phase decomposition, sign consistency, state motion, and Perron accessibility."""
from __future__ import annotations
import argparse, json, math
from pathlib import Path
import numpy as np
import pandas as pd
import matched_orientation_schedule_experiment as base
import weight1_transfer_256 as w


def cycle_data(perm):
    seen=set(); lengths=[]; fixed=[]
    for start in range(len(perm)):
        if start in seen: continue
        x=start; cyc=[]
        while x not in seen:
            seen.add(x);cyc.append(x);x=perm[x]
        lengths.append(len(cyc))
        if len(cyc)==1: fixed.append(start)
    order=1
    for L in lengths: order=math.lcm(order,L)
    return order,sorted(lengths),fixed


def motion_perm(rounds,direction):
    out=[]
    for p0 in range(128):
        p=p0
        for r in range(rounds):
            k=base.K_VALUES[r]
            q=(p-k)%128 if direction=='left' else (p+k)%128
            byte,off=divmod(q,8)
            p=8*base.routing_pi(r,byte)+off
        out.append(p)
    return out


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--results-root',type=Path,default=Path('../../results'))
    ap.add_argument('--out',type=Path,default=Path('../../results/structural_checks/revision8_additional_checks.json'))
    ap.add_argument('--contexts',type=int,default=256)
    args=ap.parse_args()
    root=args.results_root
    p=root/'weight1_256'
    summary=pd.read_csv(p/'weight1_256_summary.csv').groupby('variant').first()
    rates=pd.read_csv(p/'periodic_transfer_rates_by_context.csv').pivot(index='context_index',columns='variant',values='spectral_decay_bits_per_round')
    model=pd.read_csv(p/'model_weight1_256_by_context.csv')

    phase={v:float(row['neglog2_mean_one_step_retention'])-float(row['mean_transfer_decay_bits_per_round']) for v,row in summary.iterrows()}
    sign={}
    for a,b in [('rotor','static'),('round_only','static'),('rotor','position_only'),('round_only','position_only')]:
        d=rates[a]-rates[b]
        sign[f'{a}_minus_{b}']={'positive':int((d>0).sum()),'negative':int((d<0).sum()),'zero':int((d==0).sum()),'n':int(len(d)),'mean_bits_per_round':float(d.mean()),'median_bits_per_round':float(d.median())}

    r16=model[model['rounds']==16].groupby('variant')['max_weight1_class_log2_probability'].agg(['mean','std'])
    rv={v:{'mean_log2':float(row['mean']),'sd_log2':float(row['std'])} for v,row in r16.iterrows()}
    const=(rv['static']['sd_log2']+rv['position_only']['sd_log2'])/2
    dep=(rv['rotor']['sd_log2']+rv['round_only']['sd_log2'])/2

    eq={}
    for v in ['static','position_only','rotor','round_only']:
        gap=121.0+rv[v]['mean_log2']
        rate=float(summary.loc[v,'mean_transfer_decay_bits_per_round'])
        eq[v]={'gap_bits_at_r16':gap,'periodic_rate_bits_per_round':rate,'additional_rounds_to_121':gap/rate}
    eq['rotor_minus_static_round_saving']=eq['static']['additional_rounds_to_121']-eq['rotor']['additional_rounds_to_121']
    eq['round_only_minus_static_round_saving']=eq['static']['additional_rounds_to_121']-eq['round_only']['additional_rounds_to_121']

    transport={}
    for n in (4,16):
        transport[str(n)]={}
        for direction in ('left','right'):
            perm=motion_perm(n,direction); order,lens,fixed=cycle_data(perm)
            transport[str(n)][direction]={'order':order,'fixed_points_count':len(fixed),'fixed_points':fixed,'cycle_lengths_sorted':lens}

    ddt=base.aes_ddt(); failures=[]; mincoord=1.0; checked=0
    for ki in range(args.contexts):
        ctx=base.KeyContext.build(base.deterministic_master_key(ki))
        for variant in w.VARIANTS:
            transitions=base.build_weight1_transitions(ctx,base.SCHEDULES[variant],ddt)
            edges=w.make_edge_arrays(transitions); _,starts=w.survival_by_round(edges); start=starts[16]
            v=np.ones(128,dtype=float);v/=v.sum();last=None
            for _ in range(300):
                z=v
                for r in range(15,-1,-1): z=w.apply_T_col(edges[r],z)
                lam=float(z.sum())
                if lam<=0: break
                z/=lam
                if last is not None and abs(math.log(lam)-math.log(last))<1e-13:
                    v=z;break
                last=lam;v=z
            coord=float(v[start]); checked+=1
            if coord<=1e-15: failures.append({'context':ki,'variant':variant,'start':int(start),'coordinate':coord})
            else: mincoord=min(mincoord,coord)

    data={
      'one_step_minus_periodic_bits_per_round':phase,
      'paired_periodic_sign_consistency':sign,
      'r16_log2_class_variance':rv,
      'mean_sd_temporally_constant':const,
      'mean_sd_round_dependent':dep,
      'round_dependent_sd_reduction_fraction':1-dep/const,
      'round_equivalent_random_baseline_gap':eq,
      'combined_rotation_routing_transport_only':transport,
      'transport_interpretation':'Matrix and S-box are omitted. These orders describe only whole-state rotation plus routing and are not cipher periods.',
      'perron_max_start_check':{'contexts_checked':args.contexts,'schedule_contexts_checked':checked,'failures':failures,'all_maximizing_starts_reach_dominant_survival_mode':not failures,'min_positive_coordinate_at_max_start':None if failures else mincoord}
    }
    args.out.parent.mkdir(parents=True,exist_ok=True);args.out.write_text(json.dumps(data,indent=2)+'\n')
    print(json.dumps(data,indent=2))

if __name__=='__main__': main()
