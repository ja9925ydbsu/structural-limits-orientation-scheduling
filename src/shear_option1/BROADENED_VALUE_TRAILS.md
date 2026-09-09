# Option 1 broadened three-round value-sensitive trail analysis

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint broadens the earlier three-round search in two directions:

1. every nonzero AES DDT or LAT transition is permitted at the middle S-box layer, rather than only a selected maximum-probability / maximum-correlation transition; and
2. targeted non-branch-minimal support paths are tested after the exact branch-minimal-first search.

The evidence levels remain separate. Results called **exact within a class** exhaust that stated class. Results called **found trails** are explicit valid trails but are not claimed to be global three-round optima.

## 1. Important correction to the earlier restricted checkpoint

The earlier `restricted_three_round_trail_search.cpp` was intentionally narrow. In differential mode it selected only the first DDT-entry-4 output found for each active byte, so its differential table was never an all-DDT search.

A post-check also found one transcription error in the earlier Markdown table: the reduced `0x6f` static maximum-LAT restricted search gives 19 active S-boxes, not 18. The current broadened results below supersede the earlier three-round numerical tables for architectural comparison. The exact two-round results in `VALUE_SENSITIVE_TRAILS.md` remain unchanged.

## 2. Exact all-transition search with a branch-minimal first pair

`broadened_three_round_trail_search.cpp` enumerates every first linear relation attaining the exact byte branch number and then permits every nonzero AES transition at the middle S-box layer.

For differential characteristics the optimized cost is

`-log2(P)`,

using the actual DDT entry of each middle transition and the exact AES maximum `2^-6` at the endpoint S-boxes.

For linear trails the optimized cost is

`-log2(|C|)`,

using the actual LAT magnitude of each middle transition and the exact AES maximum correlation magnitude `2^-3` at the endpoint S-boxes.

### Full 32-cell reference

| mode | schedule | phase | best cost in class | active-byte split | interpretation |
|---|---|---:|---:|---:|---|
| linear | static | all | `2^-36` | `4 -> 4 -> 4` | exact in class |
| linear | rotor | 0 | `2^-47.830075` | `4 -> 4 -> 7` | exact in class |
| linear | rotor | 1 | `2^-48.285754` | `4 -> 4 -> 7` | exact in class |
| linear | rotor | 2 | `2^-47.700792` | `4 -> 4 -> 7` | exact in class |
| linear | rotor | 3 | `2^-47.660150` | `4 -> 4 -> 7` | exact in class |
| differential | static | all | `2^-73` | `4 -> 4 -> 4` | exact in class |
| differential | rotor | 0 | `2^-100` | `4 -> 4 -> 8` | exact in class |
| differential | rotor | 1 | `2^-100` | `4 -> 4 -> 8` | exact in class |
| differential | rotor | 2 | `2^-99` | `4 -> 4 -> 8` | exact in class |
| differential | rotor | 3 | `2^-99` | `4 -> 4 -> 8` | exact in class |

The static differential `4 -> 4 -> 4` trail requires one middle DDT-entry-2 transition, which is why its probability is `2^-73` rather than `2^-72`.

### Reduced 30-cell `0x6f` control

| mode | schedule | phase | best cost in class | active-byte split |
|---|---|---:|---:|---:|
| linear | static | all | `2^-54.192645` | `4 -> 3 -> 11` |
| linear | rotor | 0 | `2^-57.415037` | `4 -> 3 -> 12` |
| linear | rotor | 1 | `2^-57.415037` | `4 -> 3 -> 12` |
| linear | rotor | 2 | `2^-57.192645` | `4 -> 3 -> 12` |
| linear | rotor | 3 | `2^-57.415037` | `4 -> 3 -> 12` |
| differential | static | all | `2^-110` | `4 -> 3 -> 11` |
| differential | rotor | 0 | `2^-116` | `4 -> 3 -> 12` |
| differential | rotor | 1 | `2^-116` | `4 -> 3 -> 12` |
| differential | rotor | 2 | `2^-116` | `4 -> 3 -> 12` |
| differential | rotor | 3 | `2^-116` | `4 -> 3 -> 12` |

These reduced-control values are exact inside the same branch-minimal-first / all-middle-transition class. They should not be compared as global security levels.

## 3. Exact branch-minimal recurrence obstruction in the full rotor layer

A stronger structural fact emerged before any AES value is considered.

For the full static map, there are compatible consecutive branch-minimal relations, including the explicit `4 -> 4 -> 4` linear recurrence already reported.

For every full rotor phase, however, the output support of a branch-minimal `4 -> 4` relation in phase `r` never matches the input support of a branch-minimal `4 -> 4` relation in phase `r+1`.

Thus there is **no chain of two consecutive branch-minimal `4 -> 4` relations in the full rotor construction**, independent of DDT or LAT values. This explains why merely allowing weaker local AES transitions cannot restore the static `4 -> 4 -> 4` pattern under rotation.

This is a property of the present coefficient family, topology, and phase rule. It is not a theorem that rotation generally destroys low-weight recurrences.

The reduced `0x6f` control likewise has no compatible chain in which both consecutive linear relations attain its branch number 7. Its behavior is different in detail because its branch-minimal splits include `3 <-> 4` rather than only `4 <-> 4`.

## 4. Non-branch-minimal full-rotor trails found

The first-pair branch-minimal restriction is still artificial, so targeted support scans were used to identify lower-total-support paths outside that class. `fixed_middle_value_probe.cpp` then checks all values on the selected support path against the actual AES DDT or LAT.

### Linear: explicit `3 -> 8 -> 2` trails

A `3 -> 8 -> 2` support path with middle support `0xf0f0` exists for every full rotor phase and is LAT-compatible.

| rotor phase | found correlation magnitude | active S-boxes |
|---:|---:|---:|
| 0 | `2^-47.208939` | 13 |
| 1 | `2^-44.168297` | 13 |
| 2 | `2^-45.437758` | 13 |
| 3 | `2^-45.460478` | 13 |

Representative phase-1 masks are:

```text
z1 = 000000a40081fc000000000000000000   weight 3
x2 = 00000000fe26ba1b00000000b3a1b797   weight 8
z2 = 000000006c2b3b8e00000000cde2d962   weight 8
x3 = 0000000000000000005e000000360000   weight 2
```

This is an explicit valid trail with correlation magnitude `2^-44.168297`. It improves substantially on the branch-minimal-first rotor result for that phase (`2^-48.285754`), showing why nonminimal first-layer supports must be considered.

It still does not approach the full-static `2^-36` trail found above.

### Differential: explicit `3 -> 8 -> 3` trails

The analogous `3 -> 8 -> 2` support path exists at the linear-map level for the tested rotor phases, but the tested exact path is not AES-DDT-compatible. A nearby `3 -> 8 -> 3` path is DDT-compatible and gives:

| rotor phase | found characteristic probability | active S-boxes |
|---:|---:|---:|
| 0 | `2^-92` | 14 |
| 1 | `2^-91` | 14 |
| 2 | `2^-91` | 14 |
| 3 | `2^-91` | 14 |

Representative phase-1 differences are:

```text
z1 = 0000000000000000ae09000093000000   weight 3
x2 = 9db129ed00000000933a703400000000   weight 8
z2 = 6acb533600000000c06b2e6d00000000   weight 8
x3 = 0000000e000000000000008e00000059   weight 3
```

This improves on the phase-1 branch-minimal-first value `2^-100`, but it remains below the full-static `2^-73` characteristic found in the all-transition branch-minimal class.

These `3 -> 8 -> 2` and `3 -> 8 -> 3` entries are **found trails**, not proofs of the best global three-round rotor trail. Other nonminimal support patterns remain to be searched.

## 5. What the broader search changes

The earlier restricted search made the static-versus-rotor gap look larger than it really is because it excluded many useful AES transitions and all non-branch-minimal first supports.

The broadened evidence supports a narrower statement:

1. static and rotor remain identical at the exact two-round best-characteristic / best-trail level;
2. at three rounds, the full static map contains an unusually efficient `4 -> 4 -> 4` recurrence;
3. the rotor phase sequence destroys the corresponding consecutive branch-minimal support recurrence exactly;
4. allowing all AES transitions and nonminimal supports recovers substantially better rotor trails, so the rotor effect is not as large as the first restricted table suggested;
5. nevertheless, none of the rotor trails found in this broader pass reaches the static `2^-36` linear or `2^-73` differential trail;
6. these observations concern individual characteristics/trails, not differential or linear hulls and not overall security.

The result therefore remains architecturally interesting without justifying a claim that rotor scheduling is generally or cryptographically superior.

## 6. Reproduction

Compile the all-transition branch-minimal-first search:

```bash
g++ -O3 -std=c++17 broadened_three_round_trail_search.cpp -o broadened_three_round_trail_search

./broadened_three_round_trail_search linear static 0 0xff 8 10
./broadened_three_round_trail_search diff static 0 0xff 8 10
for r in 0 1 2 3; do
  ./broadened_three_round_trail_search linear rotor "$r" 0xff 8 10
  ./broadened_three_round_trail_search diff rotor "$r" 0xff 8 10
done

./broadened_three_round_trail_search linear static 0 0x6f 7 13
./broadened_three_round_trail_search diff static 0 0x6f 7 13
for r in 0 1 2 3; do
  ./broadened_three_round_trail_search linear rotor "$r" 0x6f 7 13
  ./broadened_three_round_trail_search diff rotor "$r" 0x6f 7 13
done
```

Representative nonminimal probes:

```bash
g++ -O3 -std=c++17 fixed_middle_value_probe.cpp -o fixed_middle_value_probe

# Full rotor linear 3 -> 8 -> 2, phase 1
./fixed_middle_value_probe linear rotor 1 0xff 3 2 0xf0f0

# Full rotor differential 3 -> 8 -> 3, phase 1
./fixed_middle_value_probe diff rotor 1 0xff 3 3 0x0f0f
```

## 7. Next step

The appropriate next step is a systematic global three-round optimization, preferably with a SAT/SMT/MILP or carefully bounded A*/meet-in-the-middle formulation, so that nonminimal support patterns are not selected manually. Only after a global three-round certificate or substantially tighter bound is available should the analysis extend to four rounds.
