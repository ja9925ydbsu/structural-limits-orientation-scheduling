#!/usr/bin/env python3
"""Verify the Revision-6 four-round routing composition and rotation sums."""
from __future__ import annotations
import argparse,json,math
from pathlib import Path
import matched_orientation_schedule_experiment as base

def compose_route(j:int)->int:
    x=j
    for r in range(4):
        x=base.routing_pi(r,x)
    return x

def cycles_of(perm):
    seen=set(); out=[]
    for start in range(len(perm)):
        if start in seen: continue
        cyc=[]; x=start
        while x not in seen:
            seen.add(x); cyc.append(x); x=perm[x]
        out.append(cyc)
    return out

def perm_order(cycles):
    o=1
    for c in cycles:
        o=math.lcm(o,len(c))
    return o

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--out',type=Path,default=Path('routing_geometry.json')); args=ap.parse_args()
    perm=[compose_route(j) for j in range(16)]
    cycles=cycles_of(perm)
    four_sums=[sum(base.K_VALUES[i:i+4]) for i in range(0,16,4)]
    expected=[((j << 1)&0xF)|((j>>3)&1) for j in range(16)]
    data={
      'four_round_routing_permutation':perm,
      'expected_bit_rotation_permutation':expected,
      'matches_b3b2b1b0_to_b2b1b0b3':perm==expected,
      'cycles':cycles,
      'order':perm_order(cycles),
      'routing_only_16_round_composition_identity':perm_order(cycles)==4,
      'rotation_schedule_bits':list(base.K_VALUES),
      'four_round_rotation_sums_bits':four_sums,
      'total_rotation_sum_bits':sum(base.K_VALUES),
      'within_byte_offset_realigns_each_four_round_block':all(v%8==0 for v in four_sums),
      'interpretation_boundary':'Routing and whole-state rotation are interleaved; the combined state-motion map is not asserted to be a pure rotation or to have zero routing contribution.'
    }
    args.out.parent.mkdir(parents=True,exist_ok=True); args.out.write_text(json.dumps(data,indent=2)+'\n')
    print(json.dumps(data,indent=2))
if __name__=='__main__': main()
