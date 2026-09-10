# Option 1 rotor phases 2 and 3: exact global three-round linear closure

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper under review and should not be used to revise, enlarge, or reinterpret that paper.

## Starting point

For the full 32-cell butterfly, the exact global three-round linear-mask activity minimum is 13 active S-boxes for every rotor phase. Exact optimization inside that 13-active class previously gave:

- phase 2: `3 -> 8 -> 2`, cost `45.437758` bits;
- phase 3: `3 -> 8 -> 2`, cost `45.460478` bits.

Because a 14-active trail has an absolute AES correlation-cost floor of 42 bits, 14-active competitors could still beat either value. A 15-active trail has floor 45 bits and therefore also had to be considered until a better-than-45-bit 14-active trail was found.

## Exact 14-active search

`exact_rotor_phase23_higher_active.cpp` generalizes the phase-1 exact-support/AES-LAT join. For a requested phase, total activity, and target cost it:

1. generates every positive activity split satisfying the exact branch-number and one-active-endpoint restrictions;
2. enumerates exact GF(2) relations on both sides of each common middle-byte support;
3. checks every compatible nonzero AES LAT transition;
4. prunes only when the remaining bytes cannot beat the supplied target even at the local AES maximum; and
5. reports any exact value trail below that target.

For total activity 14, the branch and endpoint restrictions leave the same 18 candidate triples used in the phase-1 proof.

## Phase 2

The previously known 13-active value was `45.437758` bits. The 14-active sweep found three classes below that value:

| split | best cost found in class |
|---|---:|
| `2 -> 8 -> 4` | `44.9052624949519` |
| `3 -> 8 -> 3` | **`44.6828700736155`** |
| `4 -> 8 -> 2` | `44.9638263874465` |

All remaining 14-active classes were exhaustively excluded below `44.6828700736155` bits. Therefore the exact best 14-active phase-2 trail has cost `44.6828700736155` bits and split `3 -> 8 -> 3`.

Since every 15-active trail costs at least `15 * 3 = 45` bits, no trail with 15 or more active S-boxes can beat this 14-active value. Combined with the exhaustive 13-active optimization and the exact global activity lower bound, this gives:

**Exact global best individual three-round linear-trail correlation magnitude for full rotor phase 2: `2^-44.6828700736155`.**

The 14-active optimum improves on the best 13-active phase-2 trail by approximately `0.7548879264` correlation-cost bits.

## Phase 3

The previously known 13-active value was `45.460478` bits. The 14-active sweep found:

| split | best cost found in class |
|---|---:|
| `2 -> 8 -> 4` | **`44.6937647147188`** |
| `3 -> 8 -> 3` | **`44.6937647147188`** |
| `4 -> 8 -> 2` | `45.2153651544425` |

All remaining 14-active classes were exhaustively excluded below `44.6937647147188` bits. Thus the exact best 14-active phase-3 cost is `44.6937647147188` bits, attained in at least the `2 -> 8 -> 4` and `3 -> 8 -> 3` classes.

Again, the 15-active floor is 45 bits, so no 15-active or higher trail can beat the 14-active optimum. Therefore:

**Exact global best individual three-round linear-trail correlation magnitude for full rotor phase 3: `2^-44.6937647147188`.**

The 14-active optimum improves on the best 13-active phase-3 trail by approximately `0.7667132853` correlation-cost bits.

## Matched static/rotor status after this checkpoint

For the full 32-cell layer, exact global individual three-round linear-trail results are now:

| schedule / phase | global optimum cost bits | magnitude | realizing activity |
|---|---:|---:|---:|
| static | `36` | `2^-36` | `4 -> 4 -> 4` (12 active) |
| rotor phase 1 | `44.168296900785705` | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| rotor phase 2 | `44.6828700736155` | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| rotor phase 3 | `44.6937647147188` | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |
| rotor phase 0 | not yet globally closed | current 13-active value `2^-47.208939` | higher-activity competitors remain |

The phase-2 and phase-3 results are a useful warning against equating minimum activity with minimum trail cost: in both phases, the globally best individual trail uses **more** active S-boxes than the 13-active minimum because the 14-active trail obtains sufficiently stronger local LAT transitions.

These are individual-trail results, not linear-hull results and not claims of end-to-end security.

## Reproduction

Compile:

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

For exact closure, run all 18 admissible 14-active splits individually against the final best cost. Splitting the runs is recommended because one all-splits invocation can exceed a short execution window even though each split is independent.

## Next target

Rotor phase 0 is now the only full-layer linear phase not globally closed. Its exact 13-active value is `47.208939` bits, so both 14- and 15-active competitors can beat it in principle. It is therefore the most computationally expensive remaining full-rotor linear case.
