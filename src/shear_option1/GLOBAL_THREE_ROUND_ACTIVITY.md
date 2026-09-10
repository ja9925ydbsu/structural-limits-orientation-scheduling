# Option 1 global three-round activity checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint separates exact support-level results from value-level AES DDT/LAT trail costs. The later value-level closure is consolidated in `EXACT_THREE_ROUND_VALUE_CHECKPOINT.md`.

## Full 32-cell reference: exact global activity result

For three consecutive S-box layers separated by two exact 128-bit linear maps, let the byte-activity split be

`a -> b -> c`.

A support path is feasible only if there is a nonzero GF(2) relation from an exact `a`-byte support to an exact `b`-byte support under the first inter-round map and a second exact relation from that same middle support to an exact `c`-byte support under the next inter-round map. For differential propagation the map is `L`; for linear masks it is `L^(-T)`.

The exhaustive support-level search gives:

| layer / schedule | propagation | exact global minimum active S-boxes | first realizing split |
|---|---|---:|---:|
| full 32-cell static | differential | **12** | `4 -> 4 -> 4` |
| full 32-cell static | linear masks | **12** | `4 -> 4 -> 4` |
| full 32-cell rotor phase 0 | differential | **13** | support-level `3 -> 8 -> 2` |
| full 32-cell rotor phase 1 | differential | **13** | support-level `3 -> 8 -> 2` |
| full 32-cell rotor phase 2 | differential | **13** | support-level `3 -> 8 -> 2` |
| full 32-cell rotor phase 3 | differential | **13** | support-level `3 -> 8 -> 2` |
| full 32-cell rotor phase 0 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 1 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 2 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 3 | linear masks | **13** | `3 -> 8 -> 2` |

Every admissible full-rotor three-round support pattern with total activity at most 12 is excluded. Exact GF(2) support witnesses exist at total 13 for all four rotor phases.

Thus, for this fixed full butterfly/shear construction and phase rule, rotor scheduling increases the exact global three-round activity minimum from 12 to 13 even though the exact two-round byte branch number remains 8 for both schedules.

This is a property of the present architecture, not a theorem about rotor scheduling in general and not a cryptographic security claim.

## Relation to the value-sensitive search

For the full static layer, the reported linear `4 -> 4 -> 4` trail with correlation magnitude `2^-36` uses the exact global activity minimum of 12 and attains the AES maximum correlation magnitude on every active S-box. Any trail with 13 or more active S-boxes has correlation cost at least 39 bits. Therefore `2^-36` is the **exact global optimum among individual three-round linear trails with free endpoint masks**.

The full-static differential `4 -> 4 -> 4` characteristic with probability `2^-73` is now also globally exact at the individual-characteristic level. `exact_static_diff_allmax_check.cpp` exhausts every activity-12 support shape that could realize an all-DDT=4 path and excludes them all. Since a 73-bit characteristic exists and every 13-active characteristic costs at least 78 bits, `2^-73` is the **exact global best individual three-round differential-characteristic probability**.

For the full rotor layer, exhaustive LAT optimization has now been completed over every globally minimum-activity 13-active support/value class. The exact optima **inside that 13-active class** are:

| rotor phase | exact 13-active split | exact best magnitude in the 13-active class |
|---:|---:|---:|
| 0 | `3 -> 8 -> 2` | `2^-47.208939` |
| 1 | `3 -> 8 -> 2` | `2^-44.168297` |
| 2 | `3 -> 8 -> 2` | `2^-45.437758` |
| 3 | `3 -> 8 -> 2` | `2^-45.460478` |

These values are no longer merely best-found 13-active trails. They are exact within the globally minimum-activity class. They are **not yet labeled exact global value-level optima**, because higher-activity trails can in principle compensate for an extra active S-box by using stronger LAT transitions. Phase 1 needs only its 14-active class checked; phases 0, 2, and 3 require 14- and 15-active competitors checked before a global value-level claim is justified.

For full rotor differential propagation, the support-level minimum remains 13. The previously tested `3 -> 8 -> 2` path was not DDT-compatible; the best explicit DDT-compatible rotor characteristics currently known use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`. Thus exact support minimum and best known differential characteristic remain separate facts.

## Reduced 30-cell `0x6f` control: completed exact results so far

The reduced control has a more complicated support structure because its byte branch number is 7 and its branch-minimal splits include `3 <-> 4`.

Completed exact support results are:

| layer / schedule | propagation | exact global minimum established so far | realizing split |
|---|---|---:|---:|
| reduced `0x6f` static | linear masks | **12** | `2 -> 6 -> 4` |
| reduced `0x6f` static | differential | **12** | exact support witness exists |
| reduced `0x6f` rotor phase 0 | linear masks | **14** | `3 -> 6 -> 5` |
| reduced `0x6f` rotor phase 1 | linear masks | **14** | `4 -> 8 -> 2` |
| reduced `0x6f` rotor phase 2 | linear masks | **14** | 14-active witness found and all totals below 14 excluded |

For reduced rotor linear phase 3, a 14-active exact-support witness has been found, but the below-14 exclusion was not completed at this checkpoint. The reduced-rotor differential global sweep is also still open. Those cases are therefore not labeled exact global minima here.

An important implementation lesson from the reduced control is that relaxed support-envelope feasibility can produce false positives: a nonzero vector can lie inside a proposed byte set without activating every byte in that set. Reduced-control claims therefore require exact-support witness extraction rather than nullspace feasibility alone.

## Evidence classification

- **Exact global activity minimum:** all smaller support totals are excluded and an exact-support GF(2) witness exists at the stated minimum.
- **Exact optimum inside the globally minimum-activity class:** every support/value assignment at that exact activity total is exhausted, but higher-activity competitors with stronger local transitions may remain.
- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including relevant higher-activity classes.
- **Found characteristic/trail:** an explicit valid path, with no claim that a better one is absent.

The full-static linear `2^-36` and differential `2^-73` results now qualify as exact global individual-trail/characteristic optima. The full-rotor linear values above qualify as exact optima inside the globally minimal 13-active class pending higher-activity competition.

## Next step

1. Check full-rotor linear phase 1 over all 14-active competitors below 44.168297 bits. This is the smallest remaining global-certification problem.
2. Check full-rotor phases 0, 2, and 3 over 14- and 15-active competitors below their current 13-active costs.
3. Search the full-rotor 13-active differential support class for all DDT-compatible assignments before concluding that 14 active S-boxes are necessary at the value level.
4. Complete reduced rotor phase-3 linear exclusion and the reduced-rotor differential global support sweep.
5. Only after these three-round value-level questions are settled should the analysis extend to four rounds.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
