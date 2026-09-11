# Option 1 full-rotor three-round differential closure

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling manuscript and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint globally closes the **individual three-round differential-characteristic** problem for the full 32-cell / 96-shear rotor construction, phases 0 through 3. It does not address differential hulls or end-to-end security.

## Cost model

For the AES S-box, every nonzero DDT entry is either 2 or 4. Thus each active S-box contributes either 7 or 6 probability-cost bits, respectively.

For a three-round characteristic with total activity `N`, the exact cost is therefore

`6*N + (# active middle-round transitions whose DDT entry is 2)`.

The endpoint S-boxes can always attain their local DDT maximum for free endpoint differences, so only the middle-round DDT compatibility must be joined explicitly with the two exact linear maps.

## 1. Exact support floor versus DDT-compatible floor

The exact global three-round **support** minimum for every full rotor phase is 13 active S-boxes, first realized at support level by `3 -> 8 -> 2`.

The exact 13-active value sweep checks all 11 structurally admissible activity triples. In every phase:

- ten triples have no exact two-map support path;
- `3 -> 8 -> 2` is the only support-feasible triple;
- every exact relation pair in that class is AES-DDT incompatible.

The support-feasible relation-pair counts are:

| rotor phase | `3 -> 8 -> 2` exact relation pairs | DDT-compatible pairs |
|---:|---:|---:|
| 0 | 36 | 0 |
| 1 | 20 | 0 |
| 2 | 36 | 0 |
| 3 | 20 | 0 |

Therefore **no 13-active differential characteristic exists in any rotor phase**, even though 13 is the exact support minimum.

This is an important distinction: support feasibility is weaker than AES-DDT value compatibility.

## 2. Exact 14-active optimization

After branch-number, one-active-endpoint, and branch-equality restrictions, 18 activity triples remain at total activity 14.

For every rotor phase, exactly three split classes have exact support paths:

- `2 -> 8 -> 4`
- `3 -> 8 -> 3`
- `4 -> 8 -> 2`

All other 15 admissible 14-active classes have no exact support path.

Exact AES-DDT optimization over the three support-feasible classes gives:

| phase | `2 -> 8 -> 4` | `3 -> 8 -> 3` | `4 -> 8 -> 2` | exact 14-active best |
|---:|---:|---:|---:|---:|
| 0 | **91** | 92 | 92 | **91** |
| 1 | 92 | **91** | 92 | **91** |
| 2 | 92 | **91** | 92 | **91** |
| 3 | **91** | **91** | 92 | **91** |

Thus every rotor phase has an explicit 14-active characteristic with probability `2^-91`.

Because the 14-active local floor is 84 bits, a 91-bit optimum means the best characteristic has seven middle-round DDT-entry-2 penalties beyond the local maximum baseline.

### Representative exact witnesses

Phase 0, `2 -> 8 -> 4`, cost 91:

```text
z1 = 9c000f00000000000000000000000000
x2 = 13b30000d5440000fef30000936c0000
z2 = 2bb80000a57800004eb2000018520000
x3 = 00410000000000610062000000dc0000
```

Phase 1, `3 -> 8 -> 3`, cost 91:

```text
z1 = 0000000000000000ae09000093000000
x2 = 9db129ed00000000933a703400000000
z2 = 6acb533600000000c06b2e6d00000000
x3 = 0000000e000000000000008e00000059
```

Phase 2, `3 -> 8 -> 3`, cost 91:

```text
z1 = 00fa0090000000d40000000000000000
x2 = 00000000c3ca9ae9000000001706c4a9
z2 = 00000000c3172ae900000000ba6af2c7
x3 = f50000005c0000005500000000000000
```

Phase 3, `2 -> 8 -> 4`, cost 91:

```text
z1 = 00000000009100e10000000000000000
x2 = 0000413e0000f56e00002e6000002a14
z2 = 000086350000fa970000e2b300008cbf
x3 = e700000000003c00d7000000ad000000
```

## 3. Exact 15-active exclusion at the 90-bit floor

A 15-active characteristic can beat the 91-bit 14-active result only at the absolute local floor

`15 * 6 = 90` bits.

Therefore every active S-box must use DDT entry 4. For the AES S-box, each nonzero input difference has exactly one output difference with DDT entry 4. The resulting nonzero-difference map is a permutation, so the all-DDT-4 condition can be checked deterministically in either the forward or reverse direction.

The exact branch/end-point restrictions leave **27 admissible 15-active activity triples**. `exact_rotor_differential_allmax15.cpp` exhausts all 27 for each of phases 0, 1, 2, and 3.

**No all-DDT-4 15-active characteristic exists in any rotor phase.**

Consequently every 15-active characteristic has cost at least 91 bits. Sixteen or more active S-boxes cost at least `16 * 6 = 96` bits and cannot improve the result.

## 4. Exact global result

Combining:

1. exact support minimum 13;
2. complete exclusion of DDT-compatible 13-active characteristics;
3. exact 14-active optimum 91 bits in every phase;
4. exhaustive exclusion of every 15-active 90-bit all-DDT-4 characteristic; and
5. the 96-bit floor for activity 16 or greater,

we obtain:

| rotor phase | exact global best individual 3-round differential probability | realizing activity |
|---:|---:|---:|
| 0 | **`2^-91`** | `2 -> 8 -> 4` (14 active) |
| 1 | **`2^-91`** | `3 -> 8 -> 3` (14 active) |
| 2 | **`2^-91`** | `3 -> 8 -> 3` (14 active) |
| 3 | **`2^-91`** | `2 -> 8 -> 4` and `3 -> 8 -> 3` both attain 91 bits |

Thus all four full-rotor phases have the same exact best individual three-round differential-characteristic probability, `2^-91`.

For comparison, the full-static exact global three-round differential optimum is `2^-73`. The 18-bit difference is an **individual-characteristic probability-cost difference for this fixed architecture**, not an 18-bit security gain and not a differential-hull result.

## 5. Architectural interpretation

The differential result sharpens the earlier support-only observation:

- static: exact support minimum 12 and exact differential optimum `2^-73` at 12 active S-boxes;
- rotor: exact support minimum 13, but the entire 13-active class is DDT-incompatible;
- rotor: the first DDT-compatible global optimum occurs at 14 active S-boxes and costs 91 bits.

Thus the rotor schedule creates two distinct effects in this fixed construction:

1. it raises the exact support floor from 12 to 13; and
2. AES DDT compatibility raises the effective best-characteristic activity from the 13-support floor to 14 active S-boxes.

This is architecture-specific finite evidence. It must not be generalized to rotor scheduling in general.

## 6. Reproduction

Compile the exact 13-/14-active optimizer:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_three_round_differential.cpp -o exact_rotor_three_round_differential
```

Example checks:

```bash
./exact_rotor_three_round_differential 0 13 3 8 2
./exact_rotor_three_round_differential 0 14 2 8 4
./exact_rotor_three_round_differential 1 14 3 8 3
```

Compile the exact 15-active all-DDT-4 checker:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_differential_allmax15.cpp -o exact_rotor_differential_allmax15
```

Example:

```bash
./exact_rotor_differential_allmax15 0 3 8 4
```

For the complete phase-by-phase verification:

```bash
python verify_rotor_global_differential.py --phase 0
python verify_rotor_global_differential.py --phase 1
python verify_rotor_global_differential.py --phase 2
python verify_rotor_global_differential.py --phase 3
```

Omitting `--phase` runs all four phases. The verifier intentionally executes support classes independently because monolithic exhaustive runs can exceed short execution windows.

## 7. Next target

With the full-layer three-round **linear and differential individual-trail/characteristic problems both globally closed**, the next architectural work should either:

- complete the still-open reduced `0x6f` three-round control cases, or
- begin a carefully bounded four-round analysis.

The current results do not establish linear or differential hull probabilities, nor end-to-end cryptographic security.
