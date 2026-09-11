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
8. exact value-level closure for the full-static cases and all four full-rotor three-round linear and differential phases.

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
- the full-static differential characteristic `2^-73` is an exact global optimum among individual three-round differential characteristics with free endpoint differences;
- full-rotor phase 1 has exact global best individual three-round linear-trail magnitude `2^-44.168296900785705`, realized by a 13-active `3 -> 8 -> 2` trail;
- full-rotor phase 2 has exact global best individual three-round linear-trail magnitude `2^-44.6828700736155`, realized by a 14-active `3 -> 8 -> 3` trail;
- full-rotor phase 3 has exact global best individual three-round linear-trail magnitude `2^-44.6937647147188`, attained in at least the 14-active `2 -> 8 -> 4` and `3 -> 8 -> 3` classes;
- full-rotor phase 0 has exact global best individual three-round linear-trail magnitude `2^-45.0156928096061`, realized by a 14-active `3 -> 8 -> 3` trail;
- for full-rotor differential propagation, the exact 13-active support minimum is not DDT-compatible in any phase: the only support-feasible class `3 -> 8 -> 2` has zero compatible AES-DDT assignments;
- the exact best 14-active differential-characteristic cost is 91 bits in every rotor phase, attained in `2 -> 8 -> 4` and/or `3 -> 8 -> 3` depending on phase;
- all 27 admissible 15-active all-DDT-4 classes are excluded in every rotor phase, so no 90-bit competitor exists;
- therefore every full-rotor phase has exact global best individual three-round differential-characteristic probability **`2^-91`**.

## Current three-round value-sensitive status

For the **full 32-cell static layer**:

- linear: exact global individual-trail optimum `2^-36`, realized by `4 -> 4 -> 4`;
- differential: exact global individual-characteristic optimum `2^-73`, realized by a 12-active `4 -> 4 -> 4` characteristic whose middle DDT entries are `4,4,4,2`.

For the **full rotor linear layer**, all four phases are globally closed:

- phase 1: `2^-44.168296900785705`, 13-active `3 -> 8 -> 2`;
- phase 2: `2^-44.6828700736155`, 14-active `3 -> 8 -> 3`;
- phase 3: `2^-44.6937647147188`, 14-active `2 -> 8 -> 4` or `3 -> 8 -> 3`;
- phase 0: `2^-45.0156928096061`, 14-active `3 -> 8 -> 3`.

For the **full rotor differential layer**, all four phases are also globally closed:

- phase 0: exact global probability `2^-91`, with a 14-active `2 -> 8 -> 4` optimum;
- phase 1: exact global probability `2^-91`, with a 14-active `3 -> 8 -> 3` optimum;
- phase 2: exact global probability `2^-91`, with a 14-active `3 -> 8 -> 3` optimum;
- phase 3: exact global probability `2^-91`, attained by at least the 14-active `2 -> 8 -> 4` and `3 -> 8 -> 3` classes.

The differential support floor remains 13, but the entire 13-active class is DDT-incompatible. Thus the effective globally optimal differential-characteristic activity is 14 for every rotor phase.

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
- `exact_rotor_phase1_14_active.cpp`, `verify_phase1_global_linear.py`: exhaustive phase-1 higher-activity linear closure.
- `exact_rotor_phase23_higher_active.cpp`, `verify_phase23_global_linear.py`, `PHASE23_GLOBAL_LINEAR.md`: exact phase-2/3 higher-activity linear closure and reproduction.
- `verify_phase0_global_linear.py`, `PHASE0_GLOBAL_LINEAR.md`: exact phase-0 higher-activity linear closure and reproduction.
- `exact_rotor_three_round_differential.cpp`: exact 13-/14-active rotor differential optimizer.
- `exact_rotor_differential_allmax15.cpp`: exact 15-active all-DDT-4 rotor differential exclusion.
- `verify_rotor_global_differential.py`, `ROTOR_GLOBAL_DIFFERENTIAL.md`: complete full-rotor three-round differential verification and checkpoint.
- `verify_three_round_value_checkpoint.py`: compile/run verification driver for the broader three-round checkpoint.
- `EXACT_THREE_ROUND_VALUE_CHECKPOINT.md`: consolidated exact three-round value-level status.

These are architectural facts and exact finite computations for the stated construction. They concern individual trails/characteristics, not linear or differential hulls and not end-to-end cryptographic security.
