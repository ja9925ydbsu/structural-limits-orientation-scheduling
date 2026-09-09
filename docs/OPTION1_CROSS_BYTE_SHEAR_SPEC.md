# Option 1 Cross-Byte Hill-Enigma Shear Redesign — Development Specification v0.1

## Status and scope

This is a fresh architecture-development track. It does **not** revise or overwrite the IJIS Revision 2 manuscript or the existing repository checkpoint. The purpose of v0.1 is to isolate and test the cross-byte shear idea before deciding whether any manuscript should be rewritten around it.

The earlier HESPN construction applied an 8x8 binary matrix independently to each byte. The new primitive keeps an 8x8 Hill-derived binary matrix but changes its role: it becomes the coefficient that injects one byte into another.

## 1. State and bit convention

The 128-bit state is

`X = (x_0, x_1, ..., x_15)`, with each `x_i` an 8-bit column vector over GF(2).

An 8x8 coefficient matrix `M` is stored as eight row bytes in MSB-first order. For byte `x`, `apply_matrix_8(M, x)` returns the byte whose bit vector is `M v(x)`.

## 2. Elementary cross-byte shear

For distinct byte positions `i` and `j`, define

`T_{i <- j,M}: x_i <- x_i XOR M x_j`, with every other byte unchanged.

Equivalently, on the ordered pair `(x_i, x_j)`,

`[x_i']   [I M] [x_i]`
`[x_j'] = [0 I] [x_j]`.

### Proposition 1 — unconditional invertibility

Every elementary shear is invertible for **every** 8x8 binary coefficient matrix `M`, including singular matrices.

Because the source byte `x_j` is unchanged,

`(x_i XOR M x_j) XOR M x_j = x_i`.

Thus `T_{i <- j,M}^{-1} = T_{i <- j,M}`. A composition of shears is inverted by applying the same shears in reverse order.

This changes the role of the old matrix-admissibility conditions. Matrix invertibility and branch number are no longer required for decryption correctness. They are candidate *diffusion-quality* conditions only.

## 3. Rotor-selected Hill-derived coefficients

For edge `e`, derive a seed `S_e` from

`SHA256(K || "SHEAR_MATRIX" || e_be16 || counter_be32)[:8]`.

The v0.1 baseline retains the historical admissibility filter as a controlled starting point: all four geometric orientations must be invertible over GF(2) and have bit branch number at least 4.

The clockwise matrix rotation is

`R(M)_{i,j} = M_{7-j,i}`.

For round `r` and edge `e`, the rotor schedule is

`M_{r,e} = R^((r+e) mod 4)(S_e)`.

The matched static control uses `M_{r,e} = S_e` for every round.

## 4. v0.1 topology: balanced directed hypercube

The 16 byte positions are identified with the 4-bit indices `0000` through `1111`. The layer has four stages. Stage `d` couples the eight disjoint pairs whose byte indices differ only in bit `d`.

There is one directed shear per pair, so the complete layer has 32 matrix applications: 8 per stage x 4 stages.

The current direction mask is `0x96699669`. In stage order the directed edges are:

- Stage 0: `1->0, 2->3, 4->5, 7->6, 8->9, 11->10, 13->12, 14->15`
- Stage 1: `0->2, 3->1, 6->4, 5->7, 10->8, 9->11, 12->14, 15->13`
- Stage 2: `4->0, 1->5, 2->6, 7->3, 8->12, 13->9, 14->10, 11->15`
- Stage 3: `0->8, 9->1, 10->2, 3->11, 12->4, 5->13, 6->14, 15->7`

This mask came from a deterministic support-only hill-climb. It was **not** selected using S-box probabilities, trail scores, ciphertext statistics, or a desired rotor outcome, and it is not claimed globally optimal.

### Exact one-active-byte temporal support

With nonzero-preserving coefficient maps, the four-stage topology has a unique temporal path to every reached byte for a one-active-byte input, so no same-byte path cancellation occurs in this restricted experiment.

The 16 starting positions reach either 5 or 8 active output bytes after one shear layer: eight positions reach 5 and eight reach 8. Therefore the exact restricted diagnostic is

`B_1 = min_{wt_byte(delta)=1} [ wt_byte(delta) + wt_byte(L delta) ] = 1 + 5 = 6`.

`B_1` is **not** the full byte branch number of the 128-bit linear layer. Computing or bounding the unrestricted minimum remains a separate task.

## 5. Experimental round v0.1

To avoid inheriting the old design accidentally, the first reference round deliberately omits the HESPN whole-state bit rotation and routing permutation.

The v0.1 round is:

1. `AddRoundKey`: `X <- X XOR rk_r`
2. `SubBytes`: apply the AES S-box independently to all 16 bytes
3. `CrossByteShear`: apply the 32 scheduled shears in stage order

Decryption performs the reverse operations:

1. reverse the 32 shears in reverse order
2. apply the inverse AES S-box
3. XOR the same round key

Round keys are domain-separated as

`rk_r = SHA256(K || "SHEAR_ROUNDKEY" || r_be16)[:16]`.

Routing, whole-state rotation, alternative S-box placement, and additional shear stages are architecture variables to be tested later rather than assumed now.

## 6. Required initial diagnostics

The reference diagnostics must establish:

- elementary shear self-inversion, including with singular coefficient matrices;
- invertibility and branch numbers of the retained coefficient families;
- full shear-layer inverse by reverse replay;
- 128-bit rank 128 of the composed linear layer;
- exact one-active-byte support for all 16 positions and all 255 nonzero byte values;
- matched static versus rotor support results;
- 1-, 4-, and 16-round encryption/decryption round trips.

## 7. Interpretation boundary

A higher one-active-byte support count is not a cryptographic security proof. The topology screen says only that the redesign removes the earlier structural defect in which the local matrix could not create a second active byte. The next questions are whether coefficient scheduling changes multi-round differential or linear structure, whether cancellations create low-support inputs outside the restricted class, and whether the added 32 matrix applications per round are justified by measurable structural benefit.
