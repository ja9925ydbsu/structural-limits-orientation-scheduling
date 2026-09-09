# Option 1 value-sensitive differential and linear trail checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint moves beyond activity-only branch-number screening by using the actual AES difference distribution table (DDT), linear approximation table (LAT), and exact 128-bit shear maps.

Two levels of evidence are kept separate:

1. an **exact two-round characteristic/trail result**; and
2. a **restricted three-round search**, exact only inside the stated search class and not a global optimum claim.

## Exact two-round value-sensitive result

Let `L` be the 128-bit differential propagation map through the shear layer. For linear masks, the corresponding forward inter-round mask map is

`T = L^(-T)`.

The checker `exact_two_round_value_trails.cpp` determines the byte branch number of both maps by exhaustive GF(2) support/rank tests.

It also reconstructs the AES S-box and exhaustively verifies two local facts:

- every nonzero DDT row and every nonzero DDT column attains entry `4`, so every nonzero middle byte difference can be connected on both sides by a local probability-`2^-6` transition;
- every nonzero LAT row and every nonzero LAT column attains Walsh magnitude `32`, so every nonzero middle byte mask can be connected on both sides by a local correlation-magnitude-`2^-3` transition.

Consequently, once a branch-minimal middle state exists, the local AES maxima can be attained independently on every active byte. For free nonzero endpoint differences or masks, the two-round activity lower bound is therefore attained exactly at the characteristic/trail level.

### Exact two-round results

| layer / schedule | differential branch `B_D` | linear-mask branch `B_L` | exact best two-round differential characteristic | exact best two-round linear-trail correlation magnitude |
|---|---:|---:|---:|---:|
| full 32-cell static | 8 | 8 | `2^-48` | `2^-24` |
| full 32-cell rotor phases 0..3 | 8 | 8 | `2^-48` | `2^-24` |
| reduced 30-cell `0x6f` static | 7 | 7 | `2^-42` | `2^-21` |
| reduced 30-cell `0x6f` rotor phases 0..3 | 7 | 7 | `2^-42` | `2^-21` |

These are exact results for the **best individual two-round differential characteristic** and the **largest-magnitude individual two-round linear trail**, with free endpoint differences/masks. They are not differential-hull or linear-hull results.

The exact result also confirms that the full layer's linear-mask branch number is 8 and the reduced `0x6f` layer's is 7, matching their differential branch numbers for the tested static and rotor phase classes. This equality is an observed exact property of the present maps, not a general theorem for arbitrary linear layers.

## Restricted three-round search

The program `restricted_three_round_trail_search.cpp` searches a deliberately narrower class so that value-level schedule effects can be exposed without presenting a heuristic result as a proof.

The class is:

1. enumerate first-layer states that attain the exact byte branch number;
2. differential mode: use an AES DDT-entry-4 transition at every active middle-round byte;
3. linear mode: enumerate AES LAT transitions with absolute Walsh value 32 at every active middle-round byte;
4. apply the exact next-round differential map `L` or linear-mask map `L^(-T)`;
5. count the resulting third-round active S-boxes.

The first and third S-box layers can independently be completed with their local AES maxima because every nonzero DDT/LAT row and column attains the relevant maximum.

The search is exhaustive **inside this restricted class only**. It does not rule out better three-round trails that use a non-branch-minimal first layer or one or more weaker local middle-round transitions to obtain a better global tradeoff.

### Differential characteristics found in the restricted class

All local active S-box transitions in these reported characteristics have probability `2^-6`.

| layer / schedule | best active S-box count found | characteristic probability for found trail |
|---|---:|---:|
| full 32-cell static | 20 | `2^-120` |
| full 32-cell rotor phase 0 | 22 | `2^-132` |
| full 32-cell rotor phase 1 | 22 | `2^-132` |
| full 32-cell rotor phase 2 | 22 | `2^-132` |
| full 32-cell rotor phase 3 | 22 | `2^-132` |
| reduced 30-cell `0x6f` static | 19 | `2^-114` |
| reduced 30-cell `0x6f` rotor phase 0 | 21 | `2^-126` |
| reduced 30-cell `0x6f` rotor phase 1 | 21 | `2^-126` |
| reduced 30-cell `0x6f` rotor phase 2 | 21 | `2^-126` |
| reduced 30-cell `0x6f` rotor phase 3 | 21 | `2^-126` |

Within this restricted class, rotor scheduling raises the best found active-S-box count by two for both the full and reduced layers. This is a schedule-sensitive value-level observation, not a proof that rotor scheduling globally improves differential security.

## Linear trails found in the restricted class

All local active S-box transitions in these reported trails have absolute Walsh value 32, hence correlation magnitude `2^-3` per active S-box.

| layer / schedule | best active S-box count found | correlation magnitude for found trail |
|---|---:|---:|
| full 32-cell static | 12 | `2^-36` |
| full 32-cell rotor phase 0 | 20 | `2^-60` |
| full 32-cell rotor phase 1 | 21 | `2^-63` |
| full 32-cell rotor phase 2 | 20 | `2^-60` |
| full 32-cell rotor phase 3 | 20 | `2^-60` |
| reduced 30-cell `0x6f` static | 18 | `2^-54` |
| reduced 30-cell `0x6f` rotor phase 0 | 20 | `2^-60` |
| reduced 30-cell `0x6f` rotor phase 1 | 20 | `2^-60` |
| reduced 30-cell `0x6f` rotor phase 2 | 20 | `2^-60` |
| reduced 30-cell `0x6f` rotor phase 3 | 20 | `2^-60` |

The most notable found trail is the full static 32-cell case. A concrete middle-mask sequence is

```text
z1 = e3980000981c00000000000000000000   weight 4
x2 = e30098000000000098001c0000000000   weight 4
z2 = b6002a00000000002a001c0000000000   weight 4
x3 = b62a00002a1c00000000000000000000   weight 4
```

Here `z1` is the first S-box output mask, `x2 = L^(-T) z1` is the second-round S-box input mask, `z2` is chosen through maximum-magnitude AES LAT transitions, and `x3 = L^(-T) z2` is the third-round S-box input mask. The endpoint masks can likewise be chosen to attain absolute Walsh value 32 on every active byte.

Thus this is an explicit valid three-round linear trail with 12 active S-boxes and correlation magnitude `2^-36`.

The rotor phases do not reproduce a comparably small trail inside the same restricted branch-minimal / maximum-local-correlation class: the best found counts are 20, 21, 20, and 20. This suggests that changing the coefficient orientation by round can disrupt a value-level recurrence present in the static map even though both schedules have the same exact two-round branch number.

That statement is intentionally narrow. It does **not** prove that the rotor construction has a better global three-round linear bound, because trails outside the restricted class have not yet been excluded.

## Interpretation

This checkpoint sharpens the earlier activity-only picture:

- at two rounds, static and rotor remain indistinguishable under the exact best-characteristic/best-trail metric for both the full and reduced layers;
- at three rounds, the actual DDT/LAT values reveal schedule-sensitive behavior that branch number alone cannot see;
- the full static layer has a particularly efficient maximum-correlation linear trail in the restricted search, despite having the stronger byte branch number `8`;
- rotor scheduling breaks that specific low-weight recurrence in the matched restricted search;
- the reduced control remains useful because it separates effects of cost/topology from effects of coefficient scheduling.

No security claim should be inferred from the found exponents alone. Differential and linear hulls, larger search classes, more rounds, and other attack families remain unaddressed.

## Reproduction

Compile the exact two-round checker:

```bash
g++ -O3 -std=c++17 exact_two_round_value_trails.cpp -o exact_two_round_value_trails

./exact_two_round_value_trails static 0 0xff
for r in 0 1 2 3; do ./exact_two_round_value_trails rotor "$r" 0xff; done

./exact_two_round_value_trails static 0 0x6f
for r in 0 1 2 3; do ./exact_two_round_value_trails rotor "$r" 0x6f; done
```

Compile the restricted three-round search:

```bash
g++ -O3 -std=c++17 restricted_three_round_trail_search.cpp -o restricted_three_round_trail_search

./restricted_three_round_trail_search diff static 0 0xff 8
./restricted_three_round_trail_search linear static 0 0xff 8

for r in 0 1 2 3; do
  ./restricted_three_round_trail_search diff rotor "$r" 0xff 8
  ./restricted_three_round_trail_search linear rotor "$r" 0xff 8
  ./restricted_three_round_trail_search diff rotor "$r" 0x6f 7
  ./restricted_three_round_trail_search linear rotor "$r" 0x6f 7
done
```

## Next search step

The next phase should broaden the three-round optimization in two controlled directions:

1. allow first-layer support sums above the branch minimum; and
2. allow lower-probability DDT transitions or lower-magnitude LAT transitions when they reduce later diffusion enough to improve the total trail weight.

A beam/A* search or a SAT/MILP/SMT formulation can then be used to obtain stronger bounds or, where feasible, exact certificates. Only after that should the search be extended to four or more rounds.
