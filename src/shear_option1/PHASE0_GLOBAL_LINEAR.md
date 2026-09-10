# Option 1 rotor phase 0: exact global three-round linear closure

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper under review and should not be used to revise, enlarge, or reinterpret that paper.

## Starting point

For the full 32-cell butterfly, the exact global three-round linear-mask activity minimum is 13 active S-boxes for every rotor phase. Exact optimization inside the phase-0 13-active class previously gave

- split: `3 -> 8 -> 2`;
- correlation-cost: `47.208939` bits;
- magnitude: `2^-47.208939`.

Because 14- and 15-active trails have absolute AES correlation-cost floors of 42 and 45 bits respectively, both activity levels could in principle beat the 13-active phase-0 value. Sixteen active S-boxes already cost at least `16 * 3 = 48` bits and therefore cannot beat it.

## Exact 14-active search

The parameterized checker `exact_rotor_phase23_higher_active.cpp` is valid for phase 0 as well as phases 2 and 3. It enumerates exact GF(2) relations on both sides of each common middle-byte support and evaluates the actual AES LAT cost of every compatible transition. Pruning is used only when the remaining active bytes cannot beat the supplied target even at the maximum AES Walsh magnitude 32.

The branch-number and one-active-endpoint restrictions leave 18 admissible 14-active triples. All 18 were checked.

Two relevant phase-0 values are:

| split | exact best cost in class below the original 13-active target |
|---|---:|
| `2 -> 8 -> 4` | `45.308474558834` |
| `3 -> 8 -> 3` | **`45.0156928096061`** |

Every other 14-active class was exhaustively excluded below `45.0156928096061` bits. Thus the exact best 14-active phase-0 trail has split `3 -> 8 -> 3` and correlation-cost

`45.0156928096061` bits.

This already improves on the best 13-active phase-0 trail by approximately `2.1932461904` correlation-cost bits.

## Exact 15-active competition

The 15-active floor is 45 bits, only `0.0156928096061` bits below the 14-active candidate. The nonzero AES LAT magnitudes are discrete: `4, 8, 12, 16, 20, 24, 28, 32`. Replacing even one maximum `|W|=32` transition by the next magnitude 28 costs

`log2(32/28) = 0.192645...` bits,

which is already larger than the entire remaining `0.0156928`-bit window. Therefore a 15-active trail can beat the candidate only if every active S-box attains the local maximum correlation magnitude.

For completeness, the checker was run on every structurally admissible 15-active split rather than relying only on that spectrum argument. The exact branch/end-point restrictions leave 27 triples. **All 27 are excluded below `45.0156928096061` bits.** Several of the largest classes contain more than a million exact relation pairs, so the closure is not a small-sample observation.

Examples of large excluded classes include:

- `2 -> 8 -> 5`: 1,218,688 exact relation pairs checked;
- `3 -> 8 -> 4`: 1,630,246 exact relation pairs checked;
- `4 -> 8 -> 3`: 1,045,606 exact relation pairs checked;
- `5 -> 8 -> 2`: 784,848 exact relation pairs checked.

No 15-active trail beats the 14-active candidate.

## Exact global result

Combining:

1. the exact global activity lower bound of 13;
2. the exact 13-active phase-0 optimum `47.208939` bits;
3. exhaustive optimization over all 18 admissible 14-active classes, giving `45.0156928096061` bits at `3 -> 8 -> 3`;
4. exhaustive exclusion of all 27 admissible 15-active classes below that cost; and
5. the 48-bit absolute floor for 16 or more active S-boxes,

we obtain:

**Exact global best individual three-round linear-trail correlation magnitude for full rotor phase 0: `2^-45.0156928096061`.**

Relative to the full-static exact global optimum `2^-36`, this is approximately `9.0156928096` additional correlation-cost bits. This is an individual-trail statement, not a 9.02-bit security gain and not a linear-hull result.

## Full matched linear picture

All full-layer three-round linear phases are now globally closed:

| schedule / phase | exact global cost bits | magnitude | realizing activity |
|---|---:|---:|---:|
| static | `36` | `2^-36` | `4 -> 4 -> 4` (12 active) |
| rotor phase 1 | `44.168296900785705` | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| rotor phase 2 | `44.6828700736155` | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| rotor phase 3 | `44.6937647147188` | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |
| rotor phase 0 | `45.0156928096061` | `2^-45.0156928096061` | `3 -> 8 -> 3` (14 active) |

The rotor phases therefore all have a substantially more expensive best individual three-round linear trail than the static control for this fixed topology, coefficient family, and scheduling rule. This remains an architecture-specific result and must not be generalized to rotor scheduling as a class or interpreted as a security proof.

## Reproduction

Compile the higher-activity checker:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_phase23_higher_active.cpp -o exact_rotor_higher_active
```

The phase-0 14-active search can be run split by split, for example:

```bash
./exact_rotor_higher_active 0 14 47.208940 2 8 4
./exact_rotor_higher_active 0 14 47.208940 3 8 3
```

After the best 14-active value is obtained, every admissible 15-active split is checked below that target, for example:

```bash
./exact_rotor_higher_active 0 15 45.0156928096061 3 8 4
./exact_rotor_higher_active 0 15 45.0156928096061 4 8 3
```

For a single verification workflow use:

```bash
python verify_phase0_global_linear.py
```

The verifier reproduces the 13-active phase-0 result, exhausts all 18 14-active classes, exhausts all 27 15-active classes below the resulting best cost, and verifies that the 16-active floor closes the remaining search.

## Next research target

With static and all four rotor phases now globally closed for individual three-round linear trails, the clean next full-layer question is the **three-round differential** side. The exact rotor support minimum is 13, but DDT compatibility has not yet been globally optimized. That problem should be settled before extending the value-level search to four rounds.
