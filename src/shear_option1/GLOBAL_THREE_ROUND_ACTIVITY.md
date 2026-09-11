# Option 1 global three-round activity checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling manuscript and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint records exact support-level results and points to the later exact value-level closures in `EXACT_THREE_ROUND_VALUE_CHECKPOINT.md`, `PHASE0_GLOBAL_LINEAR.md`, `PHASE23_GLOBAL_LINEAR.md`, and `ROTOR_GLOBAL_DIFFERENTIAL.md`.

## Full 32-cell reference: exact global activity result

For three consecutive S-box layers separated by two exact 128-bit linear maps, let the byte-activity split be `a -> b -> c`. Differential propagation uses `L`; linear masks use `L^(-T)`.

The exhaustive support-level search gives:

| layer / schedule | propagation | exact global minimum active S-boxes | first realizing split |
|---|---|---:|---:|
| full 32-cell static | differential | **12** | `4 -> 4 -> 4` |
| full 32-cell static | linear masks | **12** | `4 -> 4 -> 4` |
| full 32-cell rotor phase 0 | differential | **13** | `3 -> 8 -> 2` at support level |
| full 32-cell rotor phase 1 | differential | **13** | `3 -> 8 -> 2` at support level |
| full 32-cell rotor phase 2 | differential | **13** | `3 -> 8 -> 2` at support level |
| full 32-cell rotor phase 3 | differential | **13** | `3 -> 8 -> 2` at support level |
| full 32-cell rotor phase 0 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 1 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 2 | linear masks | **13** | `3 -> 8 -> 2` |
| full 32-cell rotor phase 3 | linear masks | **13** | `3 -> 8 -> 2` |

Every admissible full-rotor three-round support pattern with total activity at most 12 is excluded, and exact GF(2) witnesses exist at total 13 for all four rotor phases. Thus rotor scheduling raises the exact global three-round activity minimum from 12 to 13 for this fixed construction even though the exact two-round byte branch number remains 8 for both schedules.

This is an architecture-specific fact, not a theorem about rotor scheduling in general and not a cryptographic security claim.

## Relation to exact value-level results

Support minimum and trail/characteristic cost are different questions.

For the full static layer:

- linear: exact global individual three-round trail optimum `2^-36`;
- differential: exact global individual three-round characteristic optimum `2^-73`.

### Full rotor linear

The exact global full-rotor linear results are:

| rotor phase | exact global magnitude | realizing activity |
|---:|---:|---:|
| 0 | `2^-45.0156928096061` | `3 -> 8 -> 3` (14 active) |
| 1 | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| 2 | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| 3 | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |

Thus the **full-layer three-round individual linear-trail problem is globally closed for static and all four rotor phases**.

Phases 0, 2, and 3 provide concrete examples in which the globally best individual trail does not use the globally minimum number of active S-boxes.

### Full rotor differential

The exact support floor is 13, but value compatibility changes the effective optimum:

- all 11 structurally admissible 13-active triples are exhausted in every phase;
- `3 -> 8 -> 2` is the only support-feasible 13-active class, but it has zero DDT-compatible assignments in every phase;
- at 14 active S-boxes, only `2 -> 8 -> 4`, `3 -> 8 -> 3`, and `4 -> 8 -> 2` have exact support paths;
- exact DDT optimization gives a best 14-active cost of **91 bits in every phase**;
- all 27 admissible 15-active all-DDT-4 classes are excluded in every phase, so no 90-bit competitor exists;
- 16 or more active S-boxes cost at least 96 bits.

Therefore the **full-layer three-round individual differential-characteristic problem is also globally closed**:

| rotor phase | exact global differential probability | realizing activity |
|---:|---:|---:|
| 0 | `2^-91` | `2 -> 8 -> 4` (14 active) |
| 1 | `2^-91` | `3 -> 8 -> 3` (14 active) |
| 2 | `2^-91` | `3 -> 8 -> 3` (14 active) |
| 3 | `2^-91` | `2 -> 8 -> 4` and `3 -> 8 -> 3` both attain 91 bits |

The differential result is a second example of why support activity and value-level feasibility must be separated: the exact rotor support floor is 13, but the entire 13-active class is DDT-incompatible, so the globally optimal differential-characteristic activity is 14.

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

Relaxed support-envelope feasibility can produce false positives for this reduced control, so reduced-control claims require exact-support witness extraction rather than nullspace feasibility alone.

## Evidence classification

- **Exact global activity minimum:** all smaller support totals are excluded and an exact-support GF(2) witness exists at the stated minimum.
- **Exact optimum inside a fixed activity class:** every support/value assignment at that activity is exhausted, but higher-activity competitors can remain.
- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including relevant higher-activity classes.
- **Found characteristic/trail:** an explicit valid path with no claim that a better one is absent.

The full-static linear `2^-36`, full-static differential `2^-73`, all four full-rotor linear values, and all four full-rotor differential values `2^-91` qualify as exact global individual-path optima.

## Next step

1. Complete reduced rotor phase-3 linear exclusion and the reduced-rotor differential global support sweep.
2. Decide whether the reduced control warrants full value-level optimization.
3. Only after the reduced three-round controls are settled should the analysis extend to four rounds.

These are architecture-development results only, not linear-hull, differential-hull, or end-to-end security claims.
