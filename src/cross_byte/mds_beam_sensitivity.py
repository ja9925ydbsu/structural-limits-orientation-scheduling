#!/usr/bin/env python3
"""Revision-6 beam-width sensitivity check for the cross-byte MDS boundary experiment."""
from __future__ import annotations
import csv,json,time
from pathlib import Path
import mds_rotor_core as core
import mds_rotor_trails as trails

WIDTHS=(1500,3000,6000)
VARIANTS=("static","rotor","round_only","position_only","optimized")
ROUNDS=4
TOP=4
JOINT=16

def main(out=Path('mds_beam_sensitivity_results')):
    out.mkdir(parents=True,exist_ok=True); t0=time.time()
    ddt=trails.build_ddt(); lat=trails.build_lat(); rows=[]
    for width in WIDTHS:
        for variant in VARIANTS:
            sched=core.SCHEDULES[variant]
            for kind,table in (("differential",ddt),("linear",lat)):
                result=trails.beam_search_best(sched,ROUNDS,kind,table,
                    beam_width=width,top_per_active=TOP,joint_expansions=JOINT,
                    initial_mode='single_byte_all')
                # normalize likely key names robustly
                weight=result.get('best_weight',result.get('minimum_weight',result.get('weight')))
                active=result.get('best_active_sboxes',result.get('active_sboxes',result.get('cumulative_active')))
                log2=-float(weight) if weight is not None else result.get('best_log2_probability',result.get('best_log2_magnitude'))
                row={'beam_width':width,'variant':variant,'kind':kind,'rounds':ROUNDS,
                     'top_per_active':TOP,'joint_expansions':JOINT,
                     'reported_log2_probability_or_correlation':log2,
                     'reported_weight':weight,'reported_active_sboxes':active}
                # preserve useful scalar fields from result
                for k,v in result.items():
                    if isinstance(v,(str,int,float,bool)) and k not in row:
                        row[k]=v
                rows.append(row)
                (out/f'{kind}_{variant}_beam{width}.json').write_text(json.dumps(result,indent=2)+'\n')
                print(width,variant,kind,'weight',weight,'active',active,flush=True)
    # union fields
    fields=[]
    for r in rows:
        for k in r:
            if k not in fields: fields.append(k)
    with open(out/'beam_sensitivity.csv','w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(rows)
    (out/'metadata.json').write_text(json.dumps({'widths':WIDTHS,'variants':VARIANTS,'rounds':ROUNDS,'top_per_active':TOP,'joint_expansions':JOINT,'runtime_seconds':time.time()-t0},indent=2)+'\n')

if __name__=='__main__': main()
