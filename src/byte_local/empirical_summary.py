#!/usr/bin/env python3
"""Recompute Revision-6 uncertainty summaries from the standard matched rerun."""
from __future__ import annotations
import argparse,csv,json,math,statistics
from pathlib import Path
try:
    from scipy.stats import binomtest
except Exception:
    binomtest=None

def combine_group(rr, mean_field, sd_field):
    ns=[int(x['trials']) for x in rr]
    means=[float(x[mean_field]) for x in rr]
    sds=[float(x[sd_field]) for x in rr]
    N=sum(ns); mean=sum(n*m for n,m in zip(ns,means))/N
    ss=sum((n-1)*sd*sd+n*(m-mean)**2 for n,m,sd in zip(ns,means,sds))
    sd=math.sqrt(ss/(N-1)); hw=1.96*sd/math.sqrt(N)
    return {'n_pairs':N,'mean':mean,'sd':sd,'ci95_low':mean-hw,'ci95_high':mean+hw,'half_width':hw}

def ci(vals):
    m=statistics.mean(vals); sd=statistics.stdev(vals); hw=1.96*sd/math.sqrt(len(vals))
    return {'n':len(vals),'mean':m,'sd':sd,'ci95_low':m-hw,'ci95_high':m+hw}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('results_dir',type=Path)
    ap.add_argument('--out',type=Path,default=Path('statistical_corrections.json'))
    args=ap.parse_args(); r=args.results_dir
    av=list(csv.DictReader((r/'avalanche_rotor_minus_static_paired.csv').open()))
    data={'avalanche_rotor_minus_static':{},'differential_r4':{},'counter_r12':{}}
    for rnd in (4,5,8):
        data['avalanche_rotor_minus_static'][str(rnd)]=combine_group([x for x in av if int(x['rounds'])==rnd],'mean_rotor_minus_static','sd_paired_difference')
    dr=list(csv.DictReader((r/'differential_collision_screen.csv').open()))
    for variant in ('static','round_only','position_only','rotor'):
        rr=[x for x in dr if int(x['rounds'])==4 and x['variant']==variant]
        data['differential_r4'][variant]={
          'collisions':ci([float(x['collision_pairs']) for x in rr]),
          'output_weight':ci([float(x['mean_output_difference_weight']) for x in rr]),
        }
    cr=list(csv.DictReader((r/'counter_distance_by_key.csv').open()))
    for variant in ('static','position_only','round_only','rotor'):
        rr=[x for x in cr if int(x['rounds'])==12 and x['variant']==variant]
        c=ci([float(x['mean_hamming_distance']) for x in rr]); c['unit']='context-by-stride mean'; data['counter_r12'][variant]=c
    nist={'failures':34,'trials':2700,'nominal_failure_probability':0.01}
    if binomtest:
        nist['exact_two_sided_p']=binomtest(34,2700,0.01,alternative='two-sided').pvalue
        nist['one_sided_upper_p']=binomtest(34,2700,0.01,alternative='greater').pvalue
    data['nist_binomial']=nist
    args.out.parent.mkdir(parents=True,exist_ok=True); args.out.write_text(json.dumps(data,indent=2)+'\n')
    print(json.dumps(data,indent=2))
if __name__=='__main__': main()
