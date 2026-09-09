# Option 1: Cross-Byte Hill-Enigma Shear Redesign

This directory is a fresh architectural-development track. It does not modify the historical byte-local HESPN construction or the cross-byte GF(2^8) MDS boundary experiment.

**Scope boundary:** this Option 1 work is separate from the structural-limits-of-orientation-scheduling paper currently under review. Results developed here should not be folded into that paper or used to expand its claims.

## Primitive

The elementary operation is

`x_t <- x_t XOR M_k x_s`,

where each `x_i` is one byte viewed as an 8-bit vector over GF(2), and `M_k` is one of four 90-degree orientations of an 8x8 Hill-derived binary coefficient matrix.

A shear is reversible for every coefficient matrix `M_k`, including a singular one, because applying the same update twice cancels the injected term. Matrix invertibility is therefore not required for shear invertibility. It is retained here as a stronger coefficient-quality condition.

## Two-byte lifting cell

For a byte pair `(a,b)` and orientation index `k`, the baseline cell is:

1. `a <- a XOR M_k b`
2. `b <- b XOR M_(k XOR 1) a`
3. `a <- a XOR M_k b`

Decryption applies these three shears in reverse order.

For the fixed development matrix family in `option1_shear_core.py`, all four 8x8 dependency blocks of this 16x16 two-byte linear map have rank 8 for every `k in {0,1,2,3}`. Consequently, a nonzero difference in either input byte alone produces nonzero differences in both output bytes.

## Sixteen-byte topology

The reference cross-byte layer is a four-stage butterfly over byte indices `0,...,15`, using pair masks `1, 2, 4, 8`. Each stage contains eight disjoint two-byte lifting cells.

The rotor schedule is deliberately simple:

`k = (round + stage + pair_index) mod 4`.

A static control uses `k = 0`.

Because each stage pairs the currently reached support with a previously unreached dimension, a one-active-byte input expands as

`1 -> 2 -> 4 -> 8 -> 16`

for the current coefficient family. The accompanying self-test exhaustively checks all `16 * 255` one-active-byte values.

## Minimal round function

The initial research round is intentionally minimal:

1. XOR a 128-bit domain-separated round key.
2. Apply the AES S-box independently to 16 bytes.
3. Apply the cross-byte shear layer.

No historical HESPN whole-state rotation or routing permutation is inherited. The point is to measure what the shear architecture itself contributes.

## Matched controls completed so far

The development sequence is deliberately staged:

1. matched `static` versus `rotor` scheduling with identical S-box, round keys, topology, matrix seed, and round count;
2. chain, ring, three-stage butterfly, and four-stage butterfly topology controls;
3. partial-fourth-stage reduced-cost controls, retaining the same coefficient family and lifting cell;
4. exact two-round DDT/LAT characteristic/trail analysis;
5. restricted three-round trail searching as an initial value-sensitive screen; and
6. broadened three-round searching that permits every nonzero middle-layer AES transition and selected non-branch-minimal support paths.

Operation count is reported with topology results so that a larger network is not treated as superior merely because it performs more shears.

## Current structural status

The reference and exact diagnostic code establish:

- four distinct rank-8 matrix orientations;
- exact shear inversion and inverse composition;
- full encryption/decryption round trips;
- rank-8 two-byte dependency blocks;
- full 128-bit rank for the complete shear layer;
- exhaustive one-active-byte support propagation `1 -> 16` for the four-stage butterfly;
- exact four-stage-butterfly byte branch number `B_byte = 8` for static and rotor phases 0 through 3;
- exact topology branch numbers `3` (chain), `4` (ring), `6` (three-stage butterfly), and `8` (four-stage butterfly), unchanged by the tested static/rotor phase classes;
- within the controlled partial-fourth-stage family, the first exact `B_byte = 7` designs occur at 30 cells / 90 shears, while `B_byte = 8` requires the complete 32-cell / 96-shear layer;
- the selected 30-cell mask `0x6f` gives exact `B_byte = 7` and one-active-byte output support 14 for static and rotor phases 0 through 3, but loses the full reference layer's all-to-all rank-8 byte dependency property;
- exact AES S-box local maxima are DDT `4/256 = 2^-6` and Walsh magnitude `32/256 = 2^-3`;
- the exact linear-mask propagation map `L^(-T)` has byte branch number 8 for the full layer and 7 for the selected `0x6f` control;
- exact best individual two-round differential characteristics are `2^-48` (full) and `2^-42` (`0x6f`), while exact largest-magnitude individual two-round linear trails are `2^-24` (full) and `2^-21` (`0x6f`).

## Current three-round value-sensitive status

The broadened search supersedes the earlier restricted three-round tables for architectural comparison.

For the **full 32-cell static layer**, the all-transition branch-minimal-first search gives:

- linear: explicit `4 -> 4 -> 4` trail with correlation magnitude `2^-36`;
- differential: explicit `4 -> 4 -> 4` characteristic with probability `2^-73`.

For the **full rotor layer**, no two consecutive branch-minimal `4 -> 4` relations have compatible middle support in any rotor phase. This obstruction exists at the linear-map level before AES DDT/LAT values are considered.

Allowing every nonzero middle-layer AES transition gives exact branch-minimal-first rotor values of approximately:

- linear: `2^-47.66` to `2^-48.29`, with `4 -> 4 -> 7` activity;
- differential: `2^-99` to `2^-100`, with `4 -> 4 -> 8` activity.

Allowing selected **non-branch-minimal first transitions** improves the rotor trails further. Explicit full-rotor trails found include:

- linear `3 -> 8 -> 2` trails with 13 active S-boxes and correlation magnitudes from `2^-44.168297` to `2^-47.208939` across the four phases;
- differential `3 -> 8 -> 3` trails with 14 active S-boxes and probabilities `2^-91` or `2^-92`.

These are found trails, not global three-round optima. They show that broadening the search narrows the initial static-versus-rotor gap, while the unusually efficient static `4 -> 4 -> 4` recurrence remains unmatched by the rotor trails found so far.

The 30-cell `0x6f` control remains a cost/topology control rather than a replacement design. Its broadened branch-minimal-first values are recorded in `BROADENED_VALUE_TRAILS.md`.

## Files

- `STRUCTURAL_DIAGNOSTICS.md`, `exact_branch_number.cpp`: exact static-versus-rotor structural diagnostics.
- `TOPOLOGY_COMPARISON.md`, `exact_topology_comparison.cpp`: initial topology sweep.
- `REDUCED_COST_COMPARISON.md`, `exact_reduced_cost_comparison.cpp`, `scan_reduced_cost_masks.py`: reduced-cost frontier.
- `TRAIL_BOUNDS.md`, `aes_sbox_local_bounds.py`: conservative activity-derived trail bounds.
- `VALUE_SENSITIVE_TRAILS.md`, `exact_two_round_value_trails.cpp`, `restricted_three_round_trail_search.cpp`: exact two-round results and preserved historical restricted three-round checkpoint.
- `BROADENED_VALUE_TRAILS.md`, `broadened_three_round_trail_search.cpp`, `fixed_middle_value_probe.cpp`: current broadened three-round value-sensitive analysis.

These are architectural facts, exact finite computations within explicitly stated classes, and clearly labeled found-trail observations. They are not a cryptographic security proof.
