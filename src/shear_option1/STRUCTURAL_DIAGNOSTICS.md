# Option 1 matched structural diagnostics

This file records the first exact structural comparison for the cross-byte Hill-Enigma shear redesign.

**Scope boundary.** This is a separate architectural-development track on `option1-cross-byte-shear`. It is not part of, and should not be used to revise or extend, the structural-limits-of-orientation-scheduling paper currently under review.

## Byte-level branch number

For the complete 128-bit linear shear layer `L`, define

`B_byte = min_{x != 0} (wt_byte(x) + wt_byte(Lx))`.

The exact checker `exact_branch_number.cpp` treats the layer as a 128 x 128 binary linear map. For an allowed input-byte support `I` and allowed output-byte support `J`, it tests whether the submatrix consisting of output bits outside `J` and input bits inside `I` has a nontrivial nullspace. This is an exact GF(2) feasibility test, not sampling.

Support sums are tested in increasing order. Cases with more active input bytes than allowed output bytes are checked through the inverse map, using `B(L) = B(L^-1)`.

### Exact result

| schedule | round phase | full rank | one-active-byte output support | exact `B_byte` | first witness split |
|---|---:|---:|---:|---:|---:|
| static | all | 128 | 16 | **8** | 4 -> 4 |
| rotor | 0 | 128 | 16 | **8** | 4 -> 4 |
| rotor | 1 | 128 | 16 | **8** | 4 -> 4 |
| rotor | 2 | 128 | 16 | **8** | 4 -> 4 |
| rotor | 3 | 128 | 16 | **8** | 4 -> 4 |

For every schedule above, all candidate support sums 2 through 7 were exhaustively excluded before a sum-8 witness was found.

Representative witnesses are:

- static: input `e8246e80000000000000000000000000` -> output `e80000006e0000002400000080000000`;
- rotor phase 0: input `d0864880000000000000000000000000` -> output `dd000000ab0000000900000021000000`;
- rotor phase 1: input `61891e80000000000000000000000000` -> output `ae000000f4000000fd000000a7000000`;
- rotor phase 2: input `03be3880000000000000000000000000` -> output `81000000a2000000050000009e000000`;
- rotor phase 3: input `46326e80000000000000000000000000` -> output `b80000000d00000070000000d3000000`.

Each listed witness has four nonzero input bytes and four nonzero output bytes.

## Matched static-versus-rotor diagnostics

All 256 byte-to-byte 8 x 8 dependency blocks of the full layer have rank 8 for both the static layer and every rotor phase. Thus the previously observed one-active-byte expansion to all 16 output bytes is consistent with a stronger blockwise statement: every output byte depends invertibly on every single input byte when the remaining input bytes are zero.

The schedule does change coefficient-level structure:

| diagnostic | static | rotor phase 0 | rotor phase 1 | rotor phase 2 | rotor phase 3 |
|---|---:|---:|---:|---:|---:|
| orientation-use histogram over 96 shears | 64, 32, 0, 0 | 24, 24, 24, 24 | 24, 24, 24, 24 | 24, 24, 24, 24 | 24, 24, 24, 24 |
| 1-bits in 128 x 128 layer matrix | 8252 | 8080 | 8207 | 8080 | 8207 |
| bit-matrix density | 0.503662 | 0.493164 | 0.500916 | 0.493164 | 0.500916 |
| branch-8 4->4 support-pair configurations | 98 | 66 | 66 | 66 | 66 |

For the branch-8 4->4 support-pair configurations, the nullity histograms were:

- static: nullity 1: 50 pairs; nullity 2: 12 pairs; nullity 8: 36 pairs;
- every rotor phase: nullity 1: 40 pairs; nullity 2: 10 pairs; nullity 8: 16 pairs.

The reduction from 98 to 66 branch-achieving support-pair configurations is a genuine structural difference for this fixed coefficient family and butterfly topology. It is **not** by itself a cryptographic security claim, and it does not change the exact byte branch number, which remains 8.

## Interpretation

The present result is deliberately narrow:

1. the butterfly shear layer is invertible and full rank;
2. one active byte reaches all 16 output bytes;
3. the true byte-level branch number is exactly 8, not 17;
4. static and rotor schedules have the same exact branch number;
5. rotor scheduling changes coefficient-level structure and reduces the number of branch-8 support-pair configurations in this experiment, but no security advantage is inferred from that fact alone.

Accordingly, the next architecture step remains separate from the paper under review: compare chain, ring, full butterfly, and reduced-stage topologies under matched coefficients, nonlinear layer, key schedule, and operation-count reporting.

## Reproduction

From `src/shear_option1`:

```bash
g++ -O3 -std=c++17 exact_branch_number.cpp -o exact_branch_number
./exact_branch_number static 0
./exact_branch_number rotor 0
./exact_branch_number rotor 1
./exact_branch_number rotor 2
./exact_branch_number rotor 3
```

The exact branch-number checker should report exclusion of support sums 2 through 7 and `byte_branch_number=8` for each run.
