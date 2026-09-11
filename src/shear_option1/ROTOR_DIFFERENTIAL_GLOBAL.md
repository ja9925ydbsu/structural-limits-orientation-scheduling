# Option 1 full-rotor three-round differential closure

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint closes the value-level **individual three-round differential-characteristic** problem for the full 32-cell rotor construction. It does not address differential hulls, boomerang-style attacks, or end-to-end security.

## AES differential cost model

For the AES S-box, every nonzero DDT entry is 2 or 4. Therefore an `N`-active three-round characteristic has exact cost

`6*N + (# middle transitions with DDT entry 2)` bits,

because endpoint active S-boxes can attain the local maximum DDT entry 4 with free endpoint differences.

The absolute local floors are therefore:

- 13 active: 78 bits;
- 14 active: 84 bits;
- 15 active: 90 bits;
- 16 active: 96 bits.

## 1. Exact 13-active incompatibility

The exact global **support** minimum is 13 active S-boxes for every rotor phase. After the already-established branch-equality and one-active endpoint restrictions, there are 11 admissible 13-active triples.

`exact_rotor_differential_value.cpp` exhausts exact GF(2) relations on the common middle-byte support and then checks the actual AES DDT values.

For every phase:

- ten of the eleven triples have no exact common support path;
- only `3 -> 8 -> 2` is support-feasible;
- that class has zero DDT-compatible value assignments.

Exact relation-pair counts for `3 -> 8 -> 2` are:

| rotor phase | exact support relation pairs | DDT-compatible pairs |
|---:|---:|---:|
| 0 | 36 | **0** |
| 1 | 20 | **0** |
| 2 | 36 | **0** |
| 3 | 20 | **0** |

Thus the support minimum 13 is **not** realizable as an AES differential characteristic. The exact DDT-compatible activity minimum is at least 14.

## 2. Exact 14-active optimum

Explicit 14-active characteristics of cost 91 bits exist in every phase. Representative witnesses are:

### Phase 0: `2 -> 8 -> 4`, cost 91

```text
z1 = 9c000f00000000000000000000000000
x2 = 13b30000d5440000fef30000936c0000
z2 = 2bb80000a57800004eb2000018520000
x3 = 00410000000000610062000000dc0000
```

### Phase 1: `3 -> 8 -> 3`, cost 91

```text
z1 = 0000000000000000ae09000093000000
x2 = 9db129ed00000000933a703400000000
z2 = 6acb533600000000c06b2e6d00000000
x3 = 0000000e000000000000008e00000059
```

### Phase 2: `3 -> 8 -> 3`, cost 91

```text
z1 = 00fa0090000000d40000000000000000
x2 = 00000000c3ca9ae9000000001706c4a9
z2 = 00000000c3172ae900000000ba6af2c7
x3 = f50000005c0000005500000000000000
```

### Phase 3: `2 -> 8 -> 4`, cost 91

```text
z1 = 00000000009100e10000000000000000
x2 = 0000413e0000f56e00002e6000002a14
z2 = 000086350000fa970000e2b300008cbf
x3 = e700000000003c00d7000000ad000000
```

Each witness has eight active middle S-boxes with DDT-entry pattern consisting of **seven 2s and one 4**. Hence the total cost is exactly

`14*6 + 7 = 91` bits.

To determine whether a 90-bit 14-active characteristic exists, `exact_rotor_differential_allmax.cpp` exploits an AES-specific fact verified by exhaustive DDT construction: each nonzero input difference has exactly one output difference with DDT entry 4, and each nonzero output difference has exactly one inverse input difference with entry 4. Thus an all-DDT=4 middle layer is deterministic from either side.

The branch/end-point restrictions leave **18** admissible 14-active triples. All 18 are exhaustively excluded at the all-DDT=4 level for every rotor phase.

Therefore the exact best 14-active differential characteristic has probability **`2^-91`** in every phase.

## 3. Exact 15-active competition

A 15-active characteristic can beat `2^-91` only if it attains its absolute local floor `2^-90`, meaning every active S-box uses DDT entry 4.

The exact branch/end-point restrictions leave **27** admissible 15-active triples. `exact_rotor_differential_allmax.cpp` exhausts all 27 for every phase using the deterministic DDT-entry-4 map described above.

Result: **no 15-active all-DDT=4 characteristic exists in any phase.**

Consequently no 15-active characteristic can beat the 91-bit 14-active witness. Any characteristic with 16 or more active S-boxes costs at least `16*6 = 96` bits and is automatically worse.

## 4. Exact global full-rotor result

Combining the exact support lower bound, 13-active DDT incompatibility, the explicit 91-bit 14-active witnesses, exhaustive 90-bit exclusions at activities 14 and 15, and the 96-bit floor for activity 16 or greater gives:

| rotor phase | exact global best individual three-round differential characteristic | one realizing activity |
|---:|---:|---:|
| 0 | **`2^-91`** | `2 -> 8 -> 4` |
| 1 | **`2^-91`** | `3 -> 8 -> 3` |
| 2 | **`2^-91`** | `3 -> 8 -> 3` |
| 3 | **`2^-91`** | `2 -> 8 -> 4` |

Phase 3 also has a 91-bit `3 -> 8 -> 3` characteristic.

Therefore the exact DDT-compatible three-round activity minimum is **14**, even though the exact support-level minimum is **13**.

## 5. Matched static/rotor three-round differential picture

The matched full-layer individual-characteristic results are now:

| schedule | exact global best three-round differential characteristic |
|---|---:|
| static | `2^-73` |
| rotor phase 0 | `2^-91` |
| rotor phase 1 | `2^-91` |
| rotor phase 2 | `2^-91` |
| rotor phase 3 | `2^-91` |

The 18-bit difference is an **individual-characteristic cost difference for this fixed three-round architecture**, not an 18-bit security gain. Differential hulls and other attack models are not included.

## 6. Reproduction

Compile:

```bash
g++ -O3 -std=c++17 -fopenmp exact_rotor_differential_value.cpp -o exact_rotor_differential_value
g++ -O3 -std=c++17 -fopenmp exact_rotor_differential_allmax.cpp -o exact_rotor_differential_allmax
```

Representative 13-active incompatibility check:

```bash
./exact_rotor_differential_value 0 13 3 8 2 999
```

Representative 91-bit witnesses:

```bash
./exact_rotor_differential_value 0 14 2 8 4 100
./exact_rotor_differential_value 1 14 3 8 3 100
./exact_rotor_differential_value 2 14 3 8 3 100
./exact_rotor_differential_value 3 14 2 8 4 100
```

Representative 14- and 15-active 90-bit exclusions:

```bash
./exact_rotor_differential_allmax 0 14 2 8 4
./exact_rotor_differential_allmax 0 15 3 8 4
```

For the complete four-phase exhaustive workflow:

```bash
python verify_rotor_differential_global.py
```

## 7. Next research target

The full reference construction is now globally closed at three rounds for both individual linear trails and individual differential characteristics. Before extending to four rounds, the remaining three-round work is primarily the reduced `0x6f` control: reduced rotor phase-3 linear support closure and the reduced-rotor differential global sweep.

These are architecture-development results only, not differential-hull, linear-hull, or end-to-end security claims.
