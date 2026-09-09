# Option 1 value-sensitive differential and linear trail checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

## Status of this checkpoint

The exact two-round results below remain current.

The original three-round tables in this file were produced by the deliberately restricted search `restricted_three_round_trail_search.cpp`. They have now been **superseded for architectural comparison** by `BROADENED_VALUE_TRAILS.md` and `broadened_three_round_trail_search.cpp`, which permit every nonzero middle-layer AES DDT/LAT transition and also probe selected non-branch-minimal support paths.

A post-check found one transcription error in the earlier restricted Markdown table: the reduced `0x6f` static maximum-LAT restricted search gives **19 active S-boxes**, not 18. The Git history preserves the original checkpoint; the corrected and broadened interpretation is recorded in `BROADENED_VALUE_TRAILS.md`.

## Exact two-round value-sensitive result

Let `L` be the 128-bit differential propagation map through the shear layer. For linear masks, the corresponding forward inter-round mask map is

`T = L^(-T)`.

The checker `exact_two_round_value_trails.cpp` determines the byte branch number of both maps by exhaustive GF(2) support/rank tests.

It also reconstructs the AES S-box and exhaustively verifies:

- every nonzero DDT row and every nonzero DDT column attains entry `4`, so every nonzero middle byte difference can be connected on both sides by a local probability-`2^-6` transition;
- every nonzero LAT row and every nonzero LAT column attains Walsh magnitude `32`, so every nonzero middle byte mask can be connected on both sides by a local correlation-magnitude-`2^-3` transition.

Consequently, once a branch-minimal middle state exists, the local AES maxima can be attained independently on every active byte. With free nonzero endpoint differences or masks, the two-round activity lower bound is attained exactly at the individual characteristic/trail level.

| layer / schedule | differential branch `B_D` | linear-mask branch `B_L` | exact best two-round differential characteristic | exact best two-round linear-trail correlation magnitude |
|---|---:|---:|---:|---:|
| full 32-cell static | 8 | 8 | `2^-48` | `2^-24` |
| full 32-cell rotor phases 0..3 | 8 | 8 | `2^-48` | `2^-24` |
| reduced 30-cell `0x6f` static | 7 | 7 | `2^-42` | `2^-21` |
| reduced 30-cell `0x6f` rotor phases 0..3 | 7 | 7 | `2^-42` | `2^-21` |

These are exact results for the best **individual** two-round differential characteristic and largest-magnitude individual two-round linear trail with free endpoints. They are not differential-hull or linear-hull results.

The equality of differential and linear-mask branch numbers is an observed exact property of the present maps, not a general theorem for arbitrary linear layers.

## Historical restricted three-round search

`restricted_three_round_trail_search.cpp` remains in the repository as a reproducible development checkpoint. Its search class was intentionally narrow:

1. the first inter-round linear relation had to attain the exact branch number;
2. differential mode selected one DDT-entry-4 output for each active middle byte;
3. linear mode allowed only LAT transitions with absolute Walsh value 32;
4. the exact next-round linear map was then applied.

That restricted search was useful because it first exposed the static full-layer `4 -> 4 -> 4` maximum-correlation recurrence and suggested that rotor scheduling disrupted it. It was never a global three-round optimization.

The broadened follow-up confirms the recurrence effect while substantially narrowing the original static-versus-rotor gap. In particular, the current analysis now distinguishes:

- **exact two-round optima**;
- **exact three-round optima inside a branch-minimal-first / all-middle-transition class**; and
- **explicit found trails on selected non-branch-minimal support paths**.

See `BROADENED_VALUE_TRAILS.md` for the current three-round results and interpretation.

## Interpretation boundary

No exponent in this development track should be presented as a cipher security level. Individual characteristics and trails do not account for differential or linear hull aggregation, and the present work does not address algebraic, integral, invariant-subspace, related-key, implementation, or side-channel attacks.

## Reproduction

Exact two-round checker:

```bash
g++ -O3 -std=c++17 exact_two_round_value_trails.cpp -o exact_two_round_value_trails

./exact_two_round_value_trails static 0 0xff
for r in 0 1 2 3; do ./exact_two_round_value_trails rotor "$r" 0xff; done

./exact_two_round_value_trails static 0 0x6f
for r in 0 1 2 3; do ./exact_two_round_value_trails rotor "$r" 0x6f; done
```

Current broadened three-round work:

```bash
g++ -O3 -std=c++17 broadened_three_round_trail_search.cpp -o broadened_three_round_trail_search
g++ -O3 -std=c++17 fixed_middle_value_probe.cpp -o fixed_middle_value_probe
```

See `BROADENED_VALUE_TRAILS.md` for the matched commands and current result tables.
