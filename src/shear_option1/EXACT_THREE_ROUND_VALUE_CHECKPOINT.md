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

`exact_static_diff_allmax_check.cpp` exhausts the required activity-12 support shapes, up to reversal, and excludes an all-DDT=4 characteristic for each:

- `2 -> 6 -> 4`
- `2 -> 7 -> 3`
- `2 -> 8 -> 2`
- `3 -> 5 -> 4`
- `3 -> 6 -> 3`
- `4 -> 4 -> 4`

An explicit `4 -> 4 -> 4` characteristic has total cost 73 bits, with middle DDT entries `4,4,4,2`. Therefore the exact global best individual three-round differential-characteristic probability for the full static layer is **`2^-73`**. Any 13-active characteristic costs at least `13 * 6 = 78` bits and cannot beat it.

## 2. Full static linear: exact global optimum `2^-36`

The exact global three-round linear-mask activity minimum is 12, and the known static `4 -> 4 -> 4` trail attains the maximum AES correlation magnitude `2^-3` at every active S-box. Its total cost is therefore exactly 36 bits. Any 13-active trail costs at least 39 bits.

Thus the exact global best individual three-round linear-trail correlation magnitude for the full static layer is **`2^-36`**.

## 3. Full rotor linear: exact 13-active class

For every rotor phase, the exact global three-round linear-mask activity minimum is 13 active S-boxes. Exact branch-equality and one-active endpoint checks reduce the 13-active search to 11 possible positive weight triples. `exact_rotor_linear_13_active.cpp` joins exact GF(2) relations on the common middle support and evaluates every compatible nonzero AES LAT transition.

Ten of the 11 classes have no compatible value trail; only `3 -> 8 -> 2` survives. The exact optima inside the globally minimum-activity 13-active class are:

| rotor phase | exact 13-active split | exact correlation-cost bits | magnitude |
|---:|---:|---:|---:|
| 0 | `3 -> 8 -> 2` | `47.208939` | `2^-47.208939` |
| 1 | `3 -> 8 -> 2` | `44.1682969008` | `2^-44.1682969008` |
| 2 | `3 -> 8 -> 2` | `45.437758` | `2^-45.437758` |
| 3 | `3 -> 8 -> 2` | `45.460478` | `2^-45.460478` |

Phase 1 is globally closed by excluding all 14-active competitors below its 13-active value. Phases 2 and 3 are globally closed differently: each has a **better 14-active trail** than its 13-active minimum-activity trail. Phase 0 remains open.

## 4. Rotor phase 1: exact global three-round linear optimum

For rotor phase 1, the exact 13-active optimum is `44.168296900785705` correlation-cost bits, realized by `3 -> 8 -> 2`.

`exact_rotor_phase1_14_active.cpp` exhausts all 18 structurally admissible 14-active triples and finds no trail below that value. Every 15-active trail costs at least `15 * 3 = 45` bits, already larger than the candidate.

Therefore:

**The exact global best individual three-round linear-trail correlation magnitude for full rotor phase 1 is `2^-44.168296900785705`.**

Relative to the full static exact optimum `2^-36`, this is approximately `8.1682969008` additional correlation-cost bits. This is an individual-trail statement, not an 8.17-bit security gain.

## 5. Rotor phase 2: exact global three-round linear optimum

The exact minimum-activity 13-active phase-2 trail has cost `45.437758` bits. The 14-active search finds three classes below it:

| split | best cost in class |
|---|---:|
| `2 -> 8 -> 4` | `44.9052624949519` |
| `3 -> 8 -> 3` | **`44.6828700736155`** |
| `4 -> 8 -> 2` | `44.9638263874465` |

All other 14-active classes are exhaustively excluded below `44.6828700736155`. Since every 15-active trail costs at least 45 bits, no 15-active or higher trail can beat the 14-active optimum.

Therefore:

**The exact global best individual three-round linear-trail correlation magnitude for full rotor phase 2 is `2^-44.6828700736155`.**

This result is especially important methodologically: the global optimum uses 14 active S-boxes even though the exact activity minimum is 13. Stronger local LAT transitions more than compensate for the extra active S-box.

## 6. Rotor phase 3: exact global three-round linear optimum

The exact minimum-activity 13-active phase-3 trail has cost `45.460478` bits. The 14-active search finds:

| split | best cost in class |
|---|---:|
| `2 -> 8 -> 4` | **`44.6937647147188`** |
| `3 -> 8 -> 3` | **`44.6937647147188`** |
| `4 -> 8 -> 2` | `45.2153651544425` |

All remaining 14-active classes are exhaustively excluded below `44.6937647147188`. Again, the 15-active floor is 45 bits, so no higher-activity trail can improve the result.

Therefore:

**The exact global best individual three-round linear-trail correlation magnitude for full rotor phase 3 is `2^-44.6937647147188`.**

At least two 14-active activity classes attain that best cost: `2 -> 8 -> 4` and `3 -> 8 -> 3`.

## 7. Current matched full-layer linear status

| schedule / phase | exact global best three-round individual linear trail | realizing activity |
|---|---:|---:|
| static | `2^-36` | `4 -> 4 -> 4` (12 active) |
| rotor phase 1 | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| rotor phase 2 | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| rotor phase 3 | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |
| rotor phase 0 | not yet globally closed | exact 13-active value `2^-47.208939`; 14- and 15-active competition remains |

Phases 2 and 3 demonstrate why minimum active-S-box count and minimum trail cost must be treated separately. Their globally best individual trails use one more active S-box than the support minimum because the actual AES LAT values are more favorable.

## 8. Remaining full-rotor value questions

Rotor phase 0 is now the only full-layer linear phase not globally closed. Its exact 13-active cost is `47.208939` bits, so both 14- and 15-active competitors can beat it in principle; 16 active S-boxes already cost at least 48 bits and cannot.

The full-rotor differential value-level optimization also remains open. The exact differential support minimum is 13, but the previously tested `3 -> 8 -> 2` support path is not DDT-compatible. The best explicit DDT-compatible rotor characteristics currently known use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`.

## 9. Reproduction

Compile the phase-2/3 higher-activity checker:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_phase23_higher_active.cpp -o exact_rotor_phase23_higher_active
```

Representative phase-2 checks:

```bash
./exact_rotor_phase23_higher_active 2 14 45.437759 2 8 4
./exact_rotor_phase23_higher_active 2 14 45.437759 3 8 3
./exact_rotor_phase23_higher_active 2 14 45.437759 4 8 2
```

Representative phase-3 checks:

```bash
./exact_rotor_phase23_higher_active 3 14 45.460479 2 8 4
./exact_rotor_phase23_higher_active 3 14 45.460479 3 8 3
./exact_rotor_phase23_higher_active 3 14 45.460479 4 8 2
```

For an automated phase-2/3 workflow:

```bash
python verify_phase23_global_linear.py
```

Because an all-splits run may exceed a short execution window, the verifier intentionally executes the 18 independent 14-active splits separately.

## 10. Next exact target

The next full-layer linear target is rotor phase 0. It has the widest remaining higher-activity window: 14- and 15-active trails must both be searched below `47.208939` bits.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
