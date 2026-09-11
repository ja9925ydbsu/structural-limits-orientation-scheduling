# Option 1 exact three-round value checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling manuscript and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint records globally exact **individual three-round linear trails** and **individual three-round differential characteristics** for the full 32-cell / 96-shear Option 1 construction. These are not linear-hull or differential-hull bounds and are not end-to-end security claims.

## Evidence levels

- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including relevant higher-activity classes.
- **Exact optimum inside a fixed activity class:** every support/value assignment at that activity is exhausted, but higher-activity paths may still compete.
- **Found characteristic/trail:** an explicit valid path without a global optimality claim.

## 1. Full static exact results

### Differential

The exact global three-round differential activity minimum is 12. The theoretical 12-active floor is `2^-72`, but `exact_static_diff_allmax_check.cpp` excludes an all-DDT-4 characteristic for every required activity-12 support shape. An explicit `4 -> 4 -> 4` characteristic has cost 73 bits, with middle DDT entries `4,4,4,2`. Any 13-active characteristic costs at least 78 bits.

**Exact global static differential probability: `2^-73`.**

### Linear

The exact global three-round linear-mask activity minimum is 12. A static `4 -> 4 -> 4` trail attains the maximum AES correlation magnitude on every active S-box, giving the absolute 36-bit floor. Any 13-active trail costs at least 39 bits.

**Exact global static linear magnitude: `2^-36`.**

## 2. Full rotor linear closure

The exact global rotor linear-mask support minimum is 13 active S-boxes in every phase. Exact optimization inside the 13-active class gives:

| phase | exact 13-active cost | realizing split |
|---:|---:|---:|
| 0 | `47.208939` | `3 -> 8 -> 2` |
| 1 | `44.1682969008` | `3 -> 8 -> 2` |
| 2 | `45.437758` | `3 -> 8 -> 2` |
| 3 | `45.460478` | `3 -> 8 -> 2` |

Higher-activity optimization changes the global optimum in phases 0, 2, and 3.

| schedule / phase | exact global linear cost | magnitude | realizing activity |
|---|---:|---:|---:|
| static | `36` | `2^-36` | `4 -> 4 -> 4` (12 active) |
| rotor 1 | `44.168296900785705` | `2^-44.168296900785705` | `3 -> 8 -> 2` (13 active) |
| rotor 2 | `44.6828700736155` | `2^-44.6828700736155` | `3 -> 8 -> 3` (14 active) |
| rotor 3 | `44.6937647147188` | `2^-44.6937647147188` | `2 -> 8 -> 4` or `3 -> 8 -> 3` (14 active) |
| rotor 0 | `45.0156928096061` | `2^-45.0156928096061` | `3 -> 8 -> 3` (14 active) |

Thus the **full-layer three-round individual linear-trail problem is globally closed for static and all four rotor phases**.

Phases 0, 2, and 3 demonstrate that minimum active-S-box count need not identify the minimum-cost value-level trail.

## 3. Full rotor differential closure

The exact global rotor differential **support** minimum is 13 active S-boxes in every phase, first realized at support level by `3 -> 8 -> 2`.

### 13-active class

`exact_rotor_three_round_differential.cpp` exhausts all 11 structurally admissible 13-active triples. For every phase:

- ten triples have no exact support path;
- `3 -> 8 -> 2` is the only support-feasible triple;
- every exact relation pair in that class is AES-DDT incompatible.

The support-feasible relation-pair counts are 36, 20, 36, and 20 for phases 0, 1, 2, and 3 respectively, with **zero DDT-compatible assignments** in every phase.

Therefore no 13-active rotor differential characteristic exists.

### 14-active class

After exact branch/end-point restrictions, 18 activity triples remain. In every phase, only three split classes have exact support paths:

- `2 -> 8 -> 4`
- `3 -> 8 -> 3`
- `4 -> 8 -> 2`

The other 15 classes have no exact support path.

Exact DDT optimization gives:

| phase | `2 -> 8 -> 4` | `3 -> 8 -> 3` | `4 -> 8 -> 2` | exact 14-active best |
|---:|---:|---:|---:|---:|
| 0 | **91** | 92 | 92 | **91** |
| 1 | 92 | **91** | 92 | **91** |
| 2 | 92 | **91** | 92 | **91** |
| 3 | **91** | **91** | 92 | **91** |

Thus every rotor phase has an explicit 14-active characteristic with probability `2^-91`.

### 15-active class

A 15-active characteristic can beat 91 bits only at the absolute AES local floor `15 * 6 = 90` bits, which requires DDT entry 4 at every active S-box.

For the AES S-box, each nonzero input difference has exactly one output difference with DDT entry 4, and the resulting nonzero-difference map is a permutation. `exact_rotor_differential_allmax15.cpp` uses this deterministic map and exhausts all **27 admissible 15-active activity triples** in every rotor phase.

**No 15-active all-DDT-4 characteristic exists in phases 0, 1, 2, or 3.**

Consequently every 15-active characteristic costs at least 91 bits. Sixteen or more active S-boxes cost at least 96 bits.

### Exact global rotor differential result

| rotor phase | exact global differential probability | realizing activity |
|---:|---:|---:|
| 0 | **`2^-91`** | `2 -> 8 -> 4` (14 active) |
| 1 | **`2^-91`** | `3 -> 8 -> 3` (14 active) |
| 2 | **`2^-91`** | `3 -> 8 -> 3` (14 active) |
| 3 | **`2^-91`** | `2 -> 8 -> 4` and `3 -> 8 -> 3` both attain 91 bits |

Thus the **full-layer three-round individual differential-characteristic problem is globally closed for static and all four rotor phases**.

The differential result shows a second separation between support and value compatibility: the rotor support floor is 13, but the entire 13-active class is DDT-incompatible, so the globally optimal differential-characteristic activity is 14.

## 4. Matched static-versus-rotor interpretation

For this fixed topology, coefficient family, and rotor rule:

- static exact linear optimum: `2^-36`;
- rotor exact linear optima: approximately `2^-44.17` to `2^-45.02` depending on phase;
- static exact differential optimum: `2^-73`;
- rotor exact differential optimum: `2^-91` in every phase.

These are **individual-path** comparisons. The differences must not be described as security-bit gains because hull effects, additional rounds, other attacks, and the 128-bit block-size ceiling remain outside these calculations.

## 5. Reproduction

Key exact checkers and drivers:

- `exact_static_diff_allmax_check.cpp`: closes the static differential result at `2^-73`;
- `exact_rotor_linear_13_active.cpp`, `exact_rotor_phase1_14_active.cpp`, `exact_rotor_phase23_higher_active.cpp`: full-rotor linear optimization;
- `verify_phase0_global_linear.py`, `verify_phase1_global_linear.py`, `verify_phase23_global_linear.py`: linear verification workflows;
- `exact_rotor_three_round_differential.cpp`: exact 13-/14-active rotor differential optimizer;
- `exact_rotor_differential_allmax15.cpp`: exact 15-active 90-bit-floor exclusion;
- `verify_rotor_global_differential.py`: complete differential verification workflow;
- `ROTOR_GLOBAL_DIFFERENTIAL.md`: detailed differential evidence and representative witnesses.

The differential verifier can be run phase by phase:

```bash
python verify_rotor_global_differential.py --phase 0
python verify_rotor_global_differential.py --phase 1
python verify_rotor_global_differential.py --phase 2
python verify_rotor_global_differential.py --phase 3
```

Omitting `--phase` runs all four phases. Split-by-split execution is intentional because monolithic exhaustive runs can exceed short execution windows even though each class is independent.

## 6. Remaining Option 1 work

The full 32-cell three-round **individual linear and differential** problems are now globally closed. Remaining architectural work includes:

1. completing the reduced `0x6f` phase-3 linear support exclusion and reduced-rotor differential sweep;
2. deciding whether the reduced control warrants full value-level optimization; and
3. only then considering a carefully bounded four-round analysis.

These are architecture-development results only, not linear-hull, differential-hull, or end-to-end security claims.
