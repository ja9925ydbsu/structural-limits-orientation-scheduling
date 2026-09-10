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
5. restricted three-round trail searching as an initial value-sensitive screen;
6. broadened three-round searching that permits every nonzero middle-layer AES transition and selected non-branch-minimal support paths;
7. global three-round support optimization, with exact support-level results kept separate from value-level trail-cost optima; and
8. exact value-level closure for the full-static differential case plus exact LAT optimization inside the globally minimal 13-active full-rotor class.

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
- exact best individual two-round differential characteristics are `2^-48` (full) and `2^-42` (`0x6f`), while exact largest-magnitude individual two-round linear trails are `2^-24` (full) and `2^-21` (`0x6f`);
- for the full 32-cell reference, the exact global three-round activity minimum is 12 active S-boxes under static scheduling and 13 under rotor phases 0 through 3, for both differential propagation and linear-mask propagation;
- the static minimum is realized by `4 -> 4 -> 4`, while rotor minima are realized at support level by `3 -> 8 -> 2`;
- the full-static linear trail `2^-36` is an exact global optimum among individual three-round linear trails with free endpoint masks;
- the full-static differential characteristic `2^-73` is now an exact global optimum among individual three-round differential characteristics with free endpoint differences;
- for each full-rotor phase, exhaustive LAT optimization over every globally minimum-activity 13-active class leaves only `3 -> 8 -> 2`, with exact in-class costs `47.208939`, `44.168297`, `45.437758`, and `45.460478` bits for phases 0, 1, 2, and 3 respectively;
- those rotor values are exact inside the globally minimal 13-active class, but are not yet labeled exact global value-level optima because 14-active competitors, and for phases 0, 2, and 3 also 15-active competitors, could in principle have stronger local LAT transitions.

## Current three-round value-sensitive status

For the **full 32-cell static layer**:

- linear: exact global individual-trail optimum `2^-36`, realized by `4 -> 4 -> 4`;
- differential: exact global individual-characteristic optimum `2^-73`, realized by a 12-active `4 -> 4 -> 4` characteristic whose middle DDT entries are `4,4,4,2`.

For the **full rotor layer**:

- every rotor phase has exact global three-round activity minimum 13;
- exhaustive value optimization within that minimum-activity class leaves only `3 -> 8 -> 2`;
- exact 13-active correlation costs are `47.208939`, `44.168297`, `45.437758`, and `45.460478` bits for phases 0 through 3;
- higher-activity competition remains open: phase 1 needs only the 14-active class checked, while phases 0, 2, and 3 require 14- and 15-active competitors checked before a global value-level optimum can be claimed;
- at the differential support level, `3 -> 8 -> 2` realizes the 13-active minimum, but the tested support path is not DDT-compatible; the best explicit DDT-compatible characteristics found so far use `3 -> 8 -> 3` and probability `2^-91` or `2^-92`.

The 30-cell `0x6f` control remains a cost/topology control rather than a replacement design. Its completed exact global-support results and still-open reduced-rotor cases are recorded in `GLOBAL_THREE_ROUND_ACTIVITY.md`.

## Files

- `STRUCTURAL_DIAGNOSTICS.md`, `exact_branch_number.cpp`: exact static-versus-rotor structural diagnostics.
- `TOPOLOGY_COMPARISON.md`, `exact_topology_comparison.cpp`: initial topology sweep.
- `REDUCED_COST_COMPARISON.md`, `exact_reduced_cost_comparison.cpp`, `scan_reduced_cost_masks.py`: reduced-cost frontier.
- `TRAIL_BOUNDS.md`, `aes_sbox_local_bounds.py`: conservative activity-derived trail bounds.
- `VALUE_SENSITIVE_TRAILS.md`, `exact_two_round_value_trails.cpp`, `restricted_three_round_trail_search.cpp`: exact two-round results and preserved historical restricted three-round checkpoint.
- `BROADENED_VALUE_TRAILS.md`, `broadened_three_round_trail_search.cpp`, `fixed_middle_value_probe.cpp`: broadened three-round value-sensitive analysis.
- `GLOBAL_THREE_ROUND_ACTIVITY.md`: exact global three-round activity checkpoint and evidence classification.
- `exact_static_diff_allmax_check.cpp`: all-DDT=4 exclusion used to close the full-static differential optimum at `2^-73`.
- `rotor_linear_13_active_boundary_check.cpp`, `exact_rotor_linear_13_active.cpp`: boundary accounting and exact LAT optimization over the globally minimal 13-active rotor class.
- `verify_three_round_value_checkpoint.py`: compile/run verification driver for the current exact checkpoint.
- `EXACT_THREE_ROUND_VALUE_CHECKPOINT.md`: consolidated evidence classification, results, reproduction commands, and remaining higher-activity rotor competition.

These are architectural facts, exact finite computations within explicitly stated classes, globally settled support-level results where noted, and clearly labeled found-trail observations. They are not a cryptographic security proof.
