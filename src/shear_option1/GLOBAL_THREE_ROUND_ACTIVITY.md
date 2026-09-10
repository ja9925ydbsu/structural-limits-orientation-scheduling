# Option 1 global three-round activity checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint records exact support-level results. Later value-level closure is consolidated in `EXACT_THREE_ROUND_VALUE_CHECKPOINT.md`; `PHASE23_GLOBAL_LINEAR.md` records the phase-2/3 higher-activity closure.

## Full 32-cell reference: exact global activity result

For three consecutive S-box layers separated by two exact 128-bit linear maps, let the byte-activity split be `a -> b -> c`. A support path is feasible only if an exact GF(2) relation maps the first support to the middle support and a second exact relation maps that same middle support to the final support. Differential propagation uses `L`; linear masks use `L^(-T)`.

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

Every admissible full-rotor three-round support pattern with total activity at most 12 is excluded, and exact GF(2) support witnesses exist at total 13 for all four rotor phases. Thus rotor scheduling raises the exact global three-round activity minimum from 12 to 13 for this fixed construction even though the exact two-round byte branch number remains 8 for both schedules.

This is an architectural property of the present coefficient family, topology, and phase rule. It is not a theorem about rotor scheduling in general and not a cryptographic security claim.

## Relation to exact value-level results

Support minimum and trail-cost optimum are different questions.

For the full static layer:

- linear: exact global individual three-round trail optimum `2^-36`;
- differential: exact global individual three-round characteristic optimum `2^-73`.

For the full rotor linear layer, the exact minimum-activity 13-active values are:

| rotor phase | exact 13-active split | exact best magnitude in that class |
|---:|---:|---:|
| 0 | `3 -> 8 -> 2` | `2^-47.208939` |
| 1 | `3 -> 8 -> 2` | `2^-44.1682969008` |
| 2 | `3 -> 8 -> 2` | `2^-45.437758` |
| 3 | `3 -> 8 -> 2` | `2^-45.460478` |

Subsequent higher-activity optimization changes the global value-level picture:

- **phase 1:** exact global optimum `2^-44.168296900785705`, still 13-active `3 -> 8 -> 2`;
- **phase 2:** exact global optimum `2^-44.6828700736155`, using 14-active `3 -> 8 -> 3`;
- **phase 3:** exact global optimum `2^-44.6937647147188`, attained in at least the 14-active `2 -> 8 -> 4` and `3 -> 8 -> 3` classes;
- **phase 0:** still open globally; 14- and 15-active competitors remain below its current 13-active cost window.

Phases 2 and 3 therefore provide a concrete example in which the globally best individual linear trail does **not** use the globally minimum number of active S-boxes. Stronger local AES LAT transitions compensate for the extra active S-box.

For full rotor differential propagation, the support-level minimum remains 13. The previously tested `3 -> 8 -> 2` path was not DDT-compatible; the best explicit DDT-compatible rotor characteristics currently known use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`. Exact differential value-level global optimization remains open.

## Reduced 30-cell `0x6f` control: completed exact support results so far

The reduced control has a more complicated support structure because its byte branch number is 7 and its branch-minimal splits include `3 <-> 4`.

Completed exact support results are:

| layer / schedule | propagation | exact global minimum established so far | realizing split |
|---|---|---:|---:|
| reduced `0x6f` static | linear masks | **12** | `2 -> 6 -> 4` |
| reduced `0x6f` static | differential | **12** | exact support witness exists |
| reduced `0x6f` rotor phase 0 | linear masks | **14** | `3 -> 6 -> 5` |
| reduced `0x6f` rotor phase 1 | linear masks | **14** | `4 -> 8 -> 2` |
| reduced `0x6f` rotor phase 2 | linear masks | **14** | 14-active witness found and all totals below 14 excluded |

For reduced rotor linear phase 3, a 14-active exact-support witness has been found, but the below-14 exclusion was not completed at this checkpoint. The reduced-rotor differential global sweep is also still open.

An important implementation lesson from the reduced control is that relaxed support-envelope feasibility can produce false positives: a nonzero vector can lie inside a proposed byte set without activating every byte in that set. Reduced-control claims therefore require exact-support witness extraction rather than nullspace feasibility alone.

## Evidence classification

- **Exact global activity minimum:** all smaller support totals are excluded and an exact-support GF(2) witness exists at the stated minimum.
- **Exact optimum inside a fixed activity class:** every support/value assignment at that activity is exhausted, but higher-activity competitors can remain.
- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including relevant higher-activity classes.
- **Found characteristic/trail:** an explicit valid path with no claim that a better one is absent.

The full-static linear `2^-36`, full-static differential `2^-73`, and full-rotor linear phase-1/2/3 values listed above qualify as exact global individual-trail/characteristic optima. Rotor phase 0 remains open on the full-layer linear side.

## Next step

1. Close full-rotor linear phase 0 over its 14- and 15-active competition window below `47.208939` bits.
2. Search the full-rotor 13-active differential support class for all DDT-compatible assignments before concluding that 14 active S-boxes are necessary at the value level.
3. Complete reduced rotor phase-3 linear exclusion and the reduced-rotor differential global support sweep.
4. Only after these three-round value-level questions are settled should the analysis extend to four rounds.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
