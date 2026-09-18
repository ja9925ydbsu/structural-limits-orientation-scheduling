#!/usr/bin/env python3
from __future__ import annotations
import json, math
from pathlib import Path
import numpy as np
import reviewer_revision_analysis as rr
import matched_orientation_schedule_experiment as base
import weight1_transfer_256 as wt

# Post hoc diagnostic selection after inspection of the initial 2^18 calibration.
# One zero-hit cell was chosen from each schedule arm to test whether a conspicuous
# surrogate-to-fixed-key discrepancy persisted at 2^20 pairs. These cells are not a
# prospectively specified, random, or prevalence-estimating sample.
CASES=[
    (0,34,'round_only'),
    (0,119,'position_only'),
    (1,17,'static'),
    (2,0,'rotor'),
]
N=1_048_576
OUT=Path(__file__).resolve().parents[2]/'results'/'reviewer_revision'/'fixed_key_r2_zero_cell_confirmation.json'
ddt=base.aes_ddt()
records=[]
for ci,s,v in CASES:
    ctx=base.KeyContext.build(rr.key_from_label(rr.PRIMARY_LABEL,ci))
    schedule=base.SCHEDULES[v]
    tr=base.build_weight1_transitions(ctx,schedule,ddt)
    pred=float(rr.all_start_survival(wt.make_edge_arrays(tr),2)[s])
    rng=np.random.default_rng(20260911 + ci*1000+s + 999_999)
    a=rng.integers(0,256,size=(N,16),dtype=np.uint8)
    b=a.copy(); b[:,s//8] ^= np.uint8(1<<(7-(s%8)))
    sa=a; sb=b; alive=np.ones(N,dtype=bool)
    for r in range(2):
        sa=rr.round_step_np(sa,ctx,r,schedule); sb=rr.round_step_np(sb,ctx,r,schedule)
        hw=base.popcount_rows(np.bitwise_xor(sa,sb))
        alive &= (np.asarray(hw)==1)
    hits=int(alive.sum())
    # Rule-of-three upper bound for zero observations; exact Clopper-Pearson one-sided 95% is 1-0.05^(1/N)
    upper95=(1-0.05**(1/N)) if hits==0 else None
    records.append({'context':ci,'start_bit':s,'variant':v,'pairs':N,'hits':hits,'observed_probability':hits/N,
                    'markov_predicted_probability':pred,'markov_expected_hits':pred*N,
                    'zero_hit_one_sided_95_upper':upper95})
    print(records[-1], flush=True)
OUT.write_text(json.dumps({'records':records},indent=2)+'\n',encoding='utf-8')
print('wrote',OUT)
