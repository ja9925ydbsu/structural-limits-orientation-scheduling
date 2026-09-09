# Option 1 reduced-cost butterfly comparison

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

## Controlled reduced-cost family

The full reference layer has four complete butterfly stages with masks `1, 2, 4, 8`, giving 32 lifting cells and 96 byte shears.

The reduced-cost family keeps the first three stages complete and varies only how many of the eight mask-8 cells are retained in the fourth stage. A final-stage keep mask uses bit `i` for pair `(i, i XOR 8)`, `i = 0,...,7`.

This isolates cost reduction from larger topology changes: the coefficient family, lifting cell, stage order, static schedule, and rotor rule are unchanged.

## Exact branch-number frontier for the family

An exhaustive phase-0 scan of all 256 possible fourth-stage keep masks gives the following best exact byte branch number by retained-cell count.

| fourth-stage cells retained | total cells | total shears | best exact `B_byte` found |
|---:|---:|---:|---:|
| 0 | 24 | 72 | 6 |
| 1 | 25 | 75 | 6 |
| 2 | 26 | 78 | 6 |
| 3 | 27 | 81 | 6 |
| 4 | 28 | 84 | 6 |
| 5 | 29 | 87 | 6 |
| 6 | 30 | 90 | 7 |
| 7 | 31 | 93 | 7 |
| 8 | 32 | 96 | 8 |

Thus, within this controlled family, the first designs reaching `B_byte = 7` occur at 30 cells / 90 shears, while `B_byte = 8` requires the complete 32-cell / 96-shear fourth-stage butterfly.

The 30-cell case was enumerated over all `C(8,6) = 28` fourth-stage masks at phase 0:

- static: 5 masks give `B_byte = 7`, while 23 give `B_byte = 6`;
- rotor: 16 masks give `B_byte = 7`, while 12 give `B_byte = 6`.

All eight 31-cell masks give `B_byte = 7` for both static and rotor at phase 0; none reaches 8.

## Selected 30-cell reduced control

A useful matched reduced control is

`final_keep_mask = 0x6f`,

which retains final-stage pair indices `0,1,2,3,5,6` and omits pair indices `4,7`.

Its exact properties are:

| schedule | round phase | cells | shears | full rank | one-active-byte output support | byte-block rank range | exact `B_byte` |
|---|---:|---:|---:|---:|---:|---:|---:|
| static | all | 30 | 90 | 128 | 14 | 0..8 | **7** |
| rotor | 0 | 30 | 90 | 128 | 14 | 0..8 | **7** |
| rotor | 1 | 30 | 90 | 128 | 14 | 0..8 | **7** |
| rotor | 2 | 30 | 90 | 128 | 14 | 0..8 | **7** |
| rotor | 3 | 30 | 90 | 128 | 14 | 0..8 | **7** |

This saves 6 of the reference layer's 96 shears, a 6.25% reduction, while lowering the exact byte branch number from 8 to 7 and lowering one-active-byte output support from 16 to 14.

The byte-block rank range also drops from `8..8` in the full four-stage butterfly to `0..8`. Therefore the reduced layer no longer has the reference layer's property that every output byte depends invertibly on every single input byte when all other input bytes are zero.

For that reason, the 30-cell design is best treated as a cost control, not as a replacement for the 32-cell reference.

## Schedule-sensitive matched example

The mask

`final_keep_mask = 0x3f`

retains pair indices `0,...,5` and omits `6,7`. For this exact same 30-cell topology:

- static gives `B_byte = 6`;
- rotor phases 0, 1, 2, and 3 each give `B_byte = 7`.

This is a genuine topology-schedule interaction for the current coefficient family. It should **not** be interpreted as a general advantage for rotor scheduling: another fixed 30-cell topology, such as `0x6f`, gives `B_byte = 7` for both schedules, and the exhaustive phase-0 scan finds static 30-cell masks that also reach 7.

## Cost interpretation

The branch-number checkpoints now form a simple controlled ladder:

- 24 cells / 72 shears: `B_byte = 6`, one-active-byte support 8;
- 30 cells / 90 shears (`0x6f`): `B_byte = 7`, one-active-byte support 14;
- 32 cells / 96 shears: `B_byte = 8`, one-active-byte support 16 and all 256 byte-to-byte dependency blocks rank 8.

Intermediate 25-through-29-cell members improve one-active-byte support progressively but do not improve the best phase-0 branch number beyond 6. The 31-cell members improve one-active-byte support to 15 but remain at branch number 7.

Branch number divided by shear count is not used as a security metric. Operation count is reported only to expose the architectural tradeoff.

## Reproduction

Compile the exact checker:

```bash
g++ -O3 -std=c++17 exact_reduced_cost_comparison.cpp -o exact_reduced_cost_comparison
```

Representative checks:

```bash
./exact_reduced_cost_comparison static 0 0x00
./exact_reduced_cost_comparison static 0 0x6f
./exact_reduced_cost_comparison rotor 0 0x6f
./exact_reduced_cost_comparison rotor 1 0x6f
./exact_reduced_cost_comparison rotor 2 0x6f
./exact_reduced_cost_comparison rotor 3 0x6f
./exact_reduced_cost_comparison static 0 0x3f
./exact_reduced_cost_comparison rotor 0 0x3f
./exact_reduced_cost_comparison static 0 0xff
./exact_reduced_cost_comparison rotor 0 0xff
```

`scan_reduced_cost_masks.py` can enumerate all 256 fourth-stage masks for a selected schedule and round phase by repeatedly invoking the exact checker.

## Current architectural decision

Retain the 32-cell four-stage butterfly as the primary reference because it is the only member of this controlled family that simultaneously has exact `B_byte = 8`, exact `1 -> 16` one-active-byte propagation, and rank 8 for every byte-to-byte dependency block.

Retain the 30-cell `0x6f` layer as the principal reduced-cost control. It provides a clean checkpoint for asking whether later differential/linear trail diagnostics justify the additional 6 shears of the full reference.

These are exact structural results for the current fixed construction, not a cryptographic security proof.
