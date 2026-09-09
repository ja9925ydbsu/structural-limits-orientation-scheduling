# Option 1: Cross-Byte Hill-Enigma Shear Redesign

This directory is a fresh architectural-development track. It does not modify the historical byte-local HESPN construction or the cross-byte GF(2^8) MDS boundary experiment.

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

The baseline cross-byte layer is a four-stage butterfly over byte indices `0,...,15`, using pair masks `1, 2, 4, 8`. Each stage contains eight disjoint two-byte lifting cells.

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

## Immediate controls

The first matched comparison is `static` versus `rotor`, with identical S-box, round keys, topology, matrix seed, and round count. Later work should add alternative topologies before any manuscript claim is drafted.

## Current structural status

The reference code establishes:

- four distinct rank-8 matrix orientations;
- exact shear inversion;
- exact inverse composition;
- full encryption/decryption round trips;
- rank-8 two-byte dependency blocks;
- exhaustive one-active-byte support propagation for the linear layer.

These are architectural facts, not a security proof.
