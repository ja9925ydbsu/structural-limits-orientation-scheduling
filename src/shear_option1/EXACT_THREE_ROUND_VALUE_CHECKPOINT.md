# Option 1 exact three-round value checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint records the value-level conclusions reached after exact global three-round activity analysis. It distinguishes global individual-characteristic/trail optima from optima proved only inside a fixed activity class.

## Evidence levels

- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including higher-activity competitors whenever their local best possible cost could still beat the candidate.
- **Exact optimum inside a fixed activity class:** all support/value assignments at that activity are exhausted, but higher-activity trails can still compete.
- **Found characteristic/trail:** an explicit valid path with no global optimality claim.

These categories concern individual differential characteristics and individual linear trails. They do not establish differential-hull or linear-hull bounds and are not end-to-end security claims.

## 1. Full static differential: exact global optimum `2^-73`

The exact global three-round differential activity minimum for the full 32-cell static layer is 12 active S-boxes. The best theoretical 12-active probability is `2^-72`, because the maximum nonzero AES DDT entry is 4, or probability `2^-6` per active S-box.

`exact_static_diff_allmax_check.cpp` exhausts the required activity-12 support shapes, up to reversal, and excludes an all-DDT=4 characteristic for each. An explicit `4 -> 4 -> 4` characteristic has total cost 73 bits, with middle DDT entries `4,4,4,2`. Any 13-active characteristic costs at least `13 * 6 = 78` bits.

Therefore the exact global best individual three-round differential-characteristic probability for the full static layer is **`2^-73`**.

## 2. Full static linear: exact global optimum `2^-36`

The exact global three-round linear-mask activity minimum is 12, and the known static `4 -> 4 -> 4` trail attains the maximum AES correlation magnitude `2^-3` at every active S-box. Its total cost is therefore exactly 36 bits. Any 13-active trail costs at least 39 bits.

Thus the exact global best individual three-round linear-trail correlation magnitude for the full static layer is **`2^-36`**.

## 3. Full rotor minimum-activity class

For every rotor phase, the exact global three-round linear-mask activity minimum is 13 active S-boxes. Exact branch-equality and one-active endpoint checks reduce the 13-active search to 11 possible positive weight triples. `exact_rotor_linear_13_active.cpp` joins exact GF(2) relations on the common middle support and evaluates every compatible nonzero AES LAT transition.

Ten of the 11 classes have no compatible value trail; only `3 -> 8 -> 2` survives. The exact optima inside the 13-active class are:

| rotor phase | exact 13-active split | exact correlation-cost bits |
|---:|---:|---:|
| 0 | `3 -> 8 -> 2` | `47.208939` |
| 1 | `3 -> 8 -> 2` | `44.1682969008` |
| 2 | `3 -> 8 -> 2` | `45.437758` |
| 3 | `3 -> 8 -> 2` | `45.460478` |

The higher-activity searches below show that phase 1 remains optimal in the 13-active class, while phases 0, 2, and 3 obtain better global trails with 14 active S-boxes.

## 4. Exact global full-rotor linear results

### Rotor phase 1

The exact 13-active optimum is `44.168296900785705` bits, realized by `3 -> 8 -> 2`. `exact_rotor_phase1_14_active.cpp` exhausts all 18 structurally admissible 14-active triples and finds no trail below that value. Every 15-active trail costs at least 45 bits.

**Exact global phase-1 magnitude: `2^-44.168296900785705`.**

### Rotor phase 2

The exact 13-active value is `45.437758` bits. Exhaustive 14-active optimization finds a better `3 -> 8 -> 3` trail at `44.6828700736155` bits. All other 14-active classes are excluded below that value, and the 15-active floor is 45 bits.

**Exact global phase-2 magnitude: `2^-44.6828700736155`.**

### Rotor phase 3

The exact 13-active value is `45.460478` bits. Exhaustive 14-active optimization gives `44.6937647147188` bits, attained in at least the `2 -> 8 -> 4` and `3 -> 8 -> 3` classes. All remaining 14-active classes are excluded below that value, and the 15-active floor is 45 bits.

**Exact global phase-3 magnitude: `2^-44.6937647147188`.**

### Rotor phase 0

The exact 13-active value is `47.208939` bits. The complete 14-active search finds a better `3 -> 8 -> 3` trail with cost `45.0156928096061` bits. A `2 -> 8 -> 4` trail also improves on the 13-active value, at `45.308474558834` bits, but is not globally best.

All 18 admissible 14-active classes are exhausted, and no other class beats `45.0156928096061` bits.

The 15-active class required a separate exact check because its theoretical floor is 45 bits. The exact branch/end-point restrictions leave 27 admissible triples. All 27 were exhaustively excluded below `45.0156928096061`. Several of the largest classes contain roughly one million or more exact relation pairs. Sixteen-active and larger trails cannot compete because their absolute AES floor is at least `16 * 3 = 48` bits.

**Exact global phase-0 magnitude: `2^-45.0156928096061`.**

## 5. Final matched full-layer three-round linear table

| schedule / phase | exact global cost bits | exact global magnitude | realizing activity |
|---|---:|---:|---:|
| static | `36` | `2^-36` | `4 -> 4 -> 4` (12 active) |
| rotor phase 1 | `44.168296900785705` | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| rotor phase 2 | `44.6828700736155` | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| rotor phase 3 | `44.6937647147188` | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |
| rotor phase 0 | `45.0156928096061` | `2^-45.0156928096061` | `3 -> 8 -> 3` (14 active) |

Two architectural observations are now exact for this construction:

1. all four rotor phases have a more expensive best individual three-round linear trail than the static control; and
2. minimum activity does not necessarily identify the minimum-cost trail: phases 0, 2, and 3 are globally optimized by 14-active trails even though their exact activity minimum is 13.

Relative to the static `2^-36` trail, the additional individual-trail correlation costs are approximately `8.1683`, `8.6829`, `8.6938`, and `9.0157` bits for rotor phases 1, 2, 3, and 0 respectively. These differences must not be described as security gains.

## 6. Remaining full-layer value question

The full-layer **three-round linear individual-trail problem is now globally closed for static and all four rotor phases**.

The principal unresolved full-layer three-round value question is differential propagation. The exact rotor support minimum is 13, but DDT compatibility has not yet been globally optimized. The best explicit DDT-compatible rotor characteristics currently known use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`.

## 7. Reproduction

Key exact checkers and drivers are:

- `exact_static_diff_allmax_check.cpp`: closes the static differential result at `2^-73`;
- `exact_rotor_linear_13_active.cpp`: optimizes every rotor phase inside the globally minimal 13-active class;
- `exact_rotor_phase1_14_active.cpp` and `verify_phase1_global_linear.py`: close phase 1 globally;
- `exact_rotor_phase23_higher_active.cpp` and `verify_phase23_global_linear.py`: close phases 2 and 3 globally;
- `verify_phase0_global_linear.py`: reuses the parameterized higher-active checker to exhaust all 18 phase-0 14-active classes and all 27 phase-0 15-active classes.

For phase 0:

```bash
python verify_phase0_global_linear.py
```

The verification is intentionally split by activity/support class because a monolithic all-splits run can exceed a short execution window even though the individual searches are independent.

## 8. Next exact target

With the full three-round linear individual-trail analysis closed, the next exact target is the **full-rotor three-round differential problem**. The first question is whether any of the exact 13-active support paths are DDT-compatible. If not, the 14-active class should be globally optimized before moving to four rounds.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
