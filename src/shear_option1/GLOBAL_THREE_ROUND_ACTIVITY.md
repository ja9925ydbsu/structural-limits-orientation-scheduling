# Option 1 global three-round activity checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint separates exact support-level results from value-level AES DDT/LAT trail costs.

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

The global activity result sharpens the earlier value-sensitive trail checkpoint.

For the full static layer, the previously reported linear `4 -> 4 -> 4` trail with correlation magnitude `2^-36` uses exactly 12 active S-boxes. Because 12 is now the exact global activity minimum, that trail is globally activity-minimal. It is also locally maximum-correlation on every active AES S-box, so no other 12-active trail can have larger magnitude than `2^-36`.

Therefore the full-static three-round linear trail magnitude `2^-36` is an **exact global optimum among individual three-round linear trails with free endpoint masks**.

For the full rotor layer, explicit LAT-compatible `3 -> 8 -> 2` trails were already found for all four phases. They therefore attain the exact global activity minimum of 13. Their reported magnitudes are:

| rotor phase | found 13-active linear trail magnitude |
|---:|---:|
| 0 | `2^-47.208939` |
| 1 | `2^-44.168297` |
| 2 | `2^-45.437758` |
| 3 | `2^-45.460478` |

These rotor trails are globally activity-minimal but are **not yet proved globally correlation-optimal**, because other 13-active support/value assignments may have better LAT cost.

For differential propagation, the static `4 -> 4 -> 4` characteristic at `2^-73` is globally activity-minimal, but its probability is not automatically a global optimum because one middle AES transition has DDT entry 2. A different 12-active characteristic could in principle have a better product probability unless all 12-active value assignments are exhausted.

For full rotor differential propagation, the support-level minimum is 13, but the previously tested `3 -> 8 -> 2` path was not DDT-compatible; the best explicit DDT-compatible rotor characteristics found so far use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`. Thus the exact support minimum and the best currently found differential characteristic remain distinct facts.

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
- **Globally activity-minimal found trail:** the trail uses the exact minimum number of active S-boxes, but its DDT/LAT product has not necessarily been globally optimized over every value assignment.
- **Exact value-level optimum:** both the activity minimum and the local transition-cost optimum are established.
- **Found characteristic/trail:** an explicit valid path, with no claim that a better one is absent.

The full-static linear `2^-36` result now qualifies as an exact value-level optimum for an individual three-round linear trail. The full-rotor 13-active linear trails qualify as globally activity-minimal found trails pending exhaustive LAT-cost optimization over all 13-active paths.

## Next step

1. Complete reduced rotor phase-3 linear exclusion and reduced rotor differential global support sweep.
2. Enumerate all globally minimal full-rotor 13-active support paths and optimize actual AES LAT cost over them.
3. Exhaust all 12-active full-static differential assignments to determine whether `2^-73` is the exact global three-round differential-characteristic optimum.
4. Search the full-rotor 13-active differential support class for DDT-compatible assignments before concluding that 14 active S-boxes are necessary at the value level.
5. Only after these three-round value-level optima are settled should the analysis extend to four rounds.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
