#!/usr/bin/env python3
"""Verify the Revision-6 differential and linear weight-one null identities."""
from __future__ import annotations
import argparse, json, math
from pathlib import Path
import matched_orientation_schedule_experiment as base


def parity(x:int)->int:
    return x.bit_count() & 1


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--out',type=Path,default=Path('null_identity_checks.json'))
    args=ap.parse_args()
    ddt=base.aes_ddt()
    wt1=[1 << i for i in range(8)]
    probs=[]
    for a in range(1,256):
        probs.append(sum(ddt[a][b] for b in wt1)/256.0)
    mean_p=sum(probs)/255.0
    positive=[p for p in probs if p>0]

    # Direct LAT / normalized-correlation check.
    col_sums_sq=[]
    for beta in range(1,256):
        total=0.0
        for alpha in range(1,256):
            walsh=0
            for x in range(256):
                walsh += 1 if parity(alpha & x)==parity(beta & base.AES_SBOX[x]) else -1
            c=walsh/256.0
            total += c*c
        col_sums_sq.append(total)
    wt1_linear_sum=sum(col_sums_sq[b-1] for b in wt1)
    mean_linear=wt1_linear_sum/255.0

    out={
      'differential':{
        'weight_one_ddt_column_mass':sum(sum(ddt[a][b] for a in range(1,256)) for b in wt1),
        'expected_mass':8*256,
        'mean_probability_over_255_nonzero_inputs':mean_p,
        'neglog2_mean_probability_bits':-math.log2(mean_p),
        'zero_mass_nonzero_input_count':sum(p==0 for p in probs),
        'mean_neglog2_probability_over_positive_inputs':sum(-math.log2(p) for p in positive)/len(positive),
        'positive_input_count':len(positive),
      },
      'linear_squared_correlation':{
        'min_parseval_column_sum':min(col_sums_sq),
        'max_parseval_column_sum':max(col_sums_sq),
        'sum_over_eight_weight_one_output_masks':wt1_linear_sum,
        'mean_squared_correlation_retention_over_255_inputs':mean_linear,
        'neglog2_mean_squared_correlation_bits':-math.log2(mean_linear),
      }
    }
    args.out.parent.mkdir(parents=True,exist_ok=True)
    args.out.write_text(json.dumps(out,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(out,indent=2))

if __name__=='__main__':
    main()
