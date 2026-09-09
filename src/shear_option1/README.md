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

The first matched comparison was `static` versus `rotor`, with identical S-box, round keys, topology, matrix seed, and round count. The second comparison held the lifting cell and coefficient family fixed while changing only the interaction topology among chain, ring, three-stage butterfly, and four-stage butterfly networks.

A third comparison varies only the number and placement of cells retained in the fourth butterfly stage. The first three butterfly stages remain complete, so cost reduction can be studied without changing the coefficient family or lifting cell.

A fourth comparison now uses the actual AES DDT/LAT together with the exact 128-bit differential and linear-mask propagation maps. Exact two-round characteristic/trail optima are separated from a restricted three-round search so that heuristic evidence is not presented as proof.

Operation count is reported with topology results so that a larger network is not treated as superior merely because it performs more shears.

## Current structural status

The reference and exact diagnostic code establish:

- four distinct rank-8 matrix orientations;
- exact shear inversion;
- exact inverse composition;
- full encryption/decryption round trips;
- rank-8 two-byte dependency blocks;
- full 128-bit rank for the complete shear layer;
- exhaustive one-active-byte support propagation `1 -> 16` for the four-stage butterfly;
- exact four-stage-butterfly byte branch number `B_byte = 8` for the static layer and all four rotor phase classes;
- branch-number witnesses for the four-stage butterfly are necessarily of the `4 active input bytes -> 4 active output bytes` form at the optimum;
- rotor scheduling changes coefficient-level structure but does not improve the exact byte branch number in the butterfly baseline;
- the initial exact topology sweep gives branch numbers `3` (chain), `4` (ring), `6` (three-stage butterfly), and `8` (four-stage butterfly), with the same values for static and all four rotor phases;
- within the controlled partial-fourth-stage family, the first exact `B_byte = 7` designs occur at 30 cells / 90 shears, while exact `B_byte = 8` requires the complete 32-cell / 96-shear layer;
- the selected 30-cell mask `0x6f` gives exact `B_byte = 7` for static and rotor phases 0 through 3, with exact one-active-byte output support 14;
- the selected 30-cell control no longer has the full reference layer's all-to-all rank-8 byte dependency blocks, so it is retained as a cost control rather than replacing the four-stage reference;
- exact AES S-box local maxima used for trail analysis are DDT `4/256 = 2^-6` and Walsh magnitude `32/256 = 2^-3`;
- the exact linear-mask propagation map `L^(-T)` has byte branch number 8 for the full layer and 7 for the selected `0x6f` control under static and rotor phases 0 through 3;
- therefore the exact best individual two-round differential characteristics are `2^-48` (full) and `2^-42` (`0x6f`), while the exact largest-magnitude individual two-round linear trails are `2^-24` (full) and `2^-21` (`0x6f`);
- a restricted three-round value-sensitive search finds a full-static linear trail with 12 active S-boxes and correlation magnitude `2^-36`; matched rotor phases require 20 or 21 active S-boxes within that same restricted search class, which is a found schedule-sensitive effect rather than a global optimum proof;
- the corresponding restricted differential search finds 20 active S-boxes for full static versus 22 for all four rotor phases, and 19 for reduced static versus 21 for all four reduced rotor phases.

See `STRUCTURAL_DIAGNOSTICS.md` and `exact_branch_number.cpp` for the matched static-versus-rotor analysis. See `TOPOLOGY_COMPARISON.md` and `exact_topology_comparison.cpp` for the initial topology sweep. See `REDUCED_COST_COMPARISON.md`, `exact_reduced_cost_comparison.cpp`, and `scan_reduced_cost_masks.py` for the reduced-cost frontier. See `TRAIL_BOUNDS.md` and `aes_sbox_local_bounds.py` for the conservative activity-derived trail bounds. See `VALUE_SENSITIVE_TRAILS.md`, `exact_two_round_value_trails.cpp`, and `restricted_three_round_trail_search.cpp` for the first value-sensitive DDT/LAT checkpoint.

These are architectural facts, exact finite computations, and clearly labeled restricted-search observations, not a cryptographic security proof.
