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

For phases 0, 2, and 3, higher-activity competitors remain open. Phase 1 is now closed globally, as described next.

## 4. Rotor phase 1: exact global three-round linear optimum

For rotor phase 1, the exact 13-active optimum is

`44.168296900785705` correlation-cost bits,

realized by a `3 -> 8 -> 2` trail. To determine whether a 14-active trail could beat it, `exact_rotor_phase1_14_active.cpp` exhausts every structurally admissible 14-active split.

After the exact branch-number and one-active endpoint restrictions, the 18 candidate triples are:

- `2 -> 7 -> 5`, `2 -> 8 -> 4`, `2 -> 9 -> 3`, `2 -> 10 -> 2`
- `3 -> 6 -> 5`, `3 -> 7 -> 4`, `3 -> 8 -> 3`, `3 -> 9 -> 2`
- `4 -> 4 -> 6`, `4 -> 5 -> 5`, `4 -> 6 -> 4`, `4 -> 7 -> 3`, `4 -> 8 -> 2`
- `5 -> 4 -> 5`, `5 -> 5 -> 4`, `5 -> 6 -> 3`, `5 -> 7 -> 2`
- `6 -> 4 -> 4`

Every one of these classes was exhaustively checked using exact GF(2) relations and the actual AES LAT values. **No 14-active trail has cost below `44.168296900785705` bits.**

Fifteen-active trails require no further enumeration: the AES per-active-S-box correlation-cost floor is 3 bits, so every 15-active trail costs at least `15 * 3 = 45` bits, already larger than the 13-active candidate.

Therefore:

**The exact global best individual three-round linear-trail correlation magnitude for full rotor phase 1 is `2^-44.168296900785705`.**

Relative to the full static exact optimum `2^-36`, the phase-1 best individual trail has approximately `8.1682969008` additional correlation-cost bits. This is an individual-trail statement, not an 8.17-bit security gain.

## 5. Remaining full-rotor value questions

The other rotor phases are not yet globally closed at the value level:

- phase 0: exact 13-active cost `47.208939`; 14- and 15-active competitors remain to be checked;
- phase 2: exact 13-active cost `45.437758`; 14- and 15-active competitors remain to be checked;
- phase 3: exact 13-active cost `45.460478`; 14- and 15-active competitors remain to be checked.

The full-rotor differential value-level optimization also remains open. The exact differential support minimum is 13, but the previously tested `3 -> 8 -> 2` support path is not DDT-compatible. The best explicit DDT-compatible rotor characteristics currently known use 14 active S-boxes with `3 -> 8 -> 3` and probability `2^-91` or `2^-92`.

## 6. Reproduction

Compile and verify the static differential result:

```bash
g++ -O3 -std=c++17 exact_static_diff_allmax_check.cpp -o exact_static_diff_allmax_check
./exact_static_diff_allmax_check 2 6 4
./exact_static_diff_allmax_check 2 7 3
./exact_static_diff_allmax_check 2 8 2
./exact_static_diff_allmax_check 3 5 4
./exact_static_diff_allmax_check 3 6 3
./exact_static_diff_allmax_check 4 4 4
```

Compile the exact minimum-activity rotor LAT checker:

```bash
g++ -O3 -std=c++17 exact_rotor_linear_13_active.cpp -o exact_rotor_linear_13_active
./exact_rotor_linear_13_active 1
```

Compile the phase-1 14-active competitor checker with OpenMP:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_phase1_14_active.cpp -o exact_rotor_phase1_14_active
```

It can check all 18 splits in one run, or one split at a time, for example:

```bash
./exact_rotor_phase1_14_active 4 8 2
```

For a single verification workflow:

```bash
python verify_phase1_global_linear.py
```

The verifier reproduces the 13-active phase-1 value, exhausts all 18 14-active classes, verifies the 15-active 45-bit floor, and reports the exact global phase-1 result.

## 7. Next exact target

With phase 1 globally closed, the next efficient target is phase 2 or phase 3. Their 13-active costs are only slightly above the 45-bit floor for 15 active S-boxes, so 14- and 15-active competition must be handled carefully. Phase 0 has the largest open window and is correspondingly the most expensive remaining linear case.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
