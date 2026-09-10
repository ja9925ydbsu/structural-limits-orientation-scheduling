# Option 1 exact three-round value checkpoint

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise, enlarge, or reinterpret that paper.

This checkpoint records the value-level conclusions reached after the exact global three-round activity analysis. It deliberately distinguishes a globally exact individual characteristic/trail optimum from an optimum proved only inside the globally minimum-activity class.

## Evidence levels used here

- **Exact global individual-characteristic/trail optimum:** every lower-cost competitor is excluded, including competitors with more active S-boxes when their local best possible cost could still beat the candidate.
- **Exact optimum inside the globally minimum-activity class:** all support/value assignments using the exact minimum number of active S-boxes are exhausted, but a higher-activity trail could still be better because its local transitions may be stronger.
- **Found characteristic/trail:** an explicit valid path with no global optimality claim.

These categories concern individual differential characteristics and individual linear trails. They do not establish differential-hull or linear-hull bounds and are not end-to-end security claims.

## 1. Full static differential: exact global optimum `2^-73`

The exact global three-round differential activity minimum for the full 32-cell static layer is 12 active S-boxes.

For a 12-active characteristic, the theoretical best possible probability is `2^-72`, because the maximum nonzero AES S-box DDT entry is 4, corresponding to probability `2^-6` per active S-box.

`exact_static_diff_allmax_check.cpp` tests whether any globally activity-minimal support shape can use DDT entry 4 at every active middle-round byte. Using the already-established branch-number and one-active endpoint exclusions, the activity-12 shapes that must be tested up to reversal are:

- `2 -> 6 -> 4`
- `2 -> 7 -> 3`
- `2 -> 8 -> 2`
- `3 -> 5 -> 4`
- `3 -> 6 -> 3`
- `4 -> 4 -> 4`

All are excluded in the all-DDT=4 test. Thus no 12-active `2^-72` characteristic exists.

The all-transition exact search already supplies an explicit static `4 -> 4 -> 4` characteristic with total cost 73 bits. Its four middle AES transitions have DDT entries `4, 4, 4, 2`; the endpoint active S-boxes can use DDT entry 4.

Therefore:

**The exact global best individual three-round differential-characteristic probability for the full static layer is `2^-73`.**

No characteristic with 13 or more active S-boxes can beat this value, because even at the local AES maximum such a characteristic costs at least `13 * 6 = 78` bits.

## 2. Full static linear: preserved exact global optimum `2^-36`

The exact global three-round linear-mask activity minimum for the full static layer is 12 active S-boxes, and the known `4 -> 4 -> 4` trail uses maximum AES correlation magnitude `2^-3` at every active S-box.

Therefore its magnitude `2^-36` meets the absolute local lower-cost bound for 12 active S-boxes. Any trail with 13 or more active S-boxes has cost at least 39 bits.

Thus the full-static value remains:

**exact global best individual three-round linear-trail correlation magnitude `2^-36`.**

## 3. Full rotor linear: exact optimization inside the global 13-active class

For every rotor phase, the exact global three-round linear-mask activity minimum is 13 active S-boxes.

The boundary checker establishes that one active byte expands to all 16 bytes under both `T = L^(-T)` and `T^(-1)`, and that branch-equality sum 8 is impossible for `1 <-> 7`, `2 <-> 6`, and `3 <-> 5`; only `4 <-> 4` remains possible at equality.

After these exact/relaxed-safe prunings, exactly 11 positive 13-active weight triples remain:

- `2 -> 7 -> 4`
- `2 -> 8 -> 3`
- `2 -> 9 -> 2`
- `3 -> 6 -> 4`
- `3 -> 7 -> 3`
- `3 -> 8 -> 2`
- `4 -> 4 -> 5`
- `4 -> 5 -> 4`
- `4 -> 6 -> 3`
- `4 -> 7 -> 2`
- `5 -> 4 -> 4`

`exact_rotor_linear_13_active.cpp` joins exact GF(2) relations on the shared middle-byte support and evaluates every compatible nonzero AES LAT transition. Across phases 0 through 3, ten of the eleven candidate split classes have no compatible value trail; only `3 -> 8 -> 2` survives.

The exact optima **within the globally minimum-activity 13-active class** are:

| rotor phase | exact 13-active split | exact best correlation magnitude in that class |
|---:|---:|---:|
| 0 | `3 -> 8 -> 2` | `2^-47.208939` |
| 1 | `3 -> 8 -> 2` | `2^-44.168297` |
| 2 | `3 -> 8 -> 2` | `2^-45.437758` |
| 3 | `3 -> 8 -> 2` | `2^-45.460478` |

These four values upgrade the earlier "best found 13-active" trails to **exact values inside the globally minimum-activity class**.

They are **not yet exact global three-round linear-trail optima**, because a trail with more than 13 active S-boxes can in principle have stronger local LAT transitions and a smaller total correlation cost.

The remaining competitor classes that can beat the current 13-active values are sharply bounded by the AES per-active-S-box minimum cost of 3 bits:

- phase 0 (`47.208939` bits): 14- and 15-active trails must still be excluded; 16 active already costs at least 48 bits;
- phase 1 (`44.168297` bits): only 14-active trails can still beat it; 15 active costs at least 45 bits;
- phase 2 (`45.437758` bits): 14- and 15-active trails must still be excluded;
- phase 3 (`45.460478` bits): 14- and 15-active trails must still be excluded.

Thus phase 1 is the smallest remaining global-certification problem.

## 4. Matched interpretation

The exact three-round comparison currently supports the following narrow conclusions for the fixed full butterfly/shear architecture and coefficient family:

1. static differential: exact global individual-characteristic optimum `2^-73`;
2. static linear: exact global individual-trail optimum `2^-36`;
3. rotor linear: exact global activity minimum 13, with exact value-level optima inside that 13-active class ranging from `2^-44.168297` to `2^-47.208939`;
4. rotor scheduling therefore destroys the static 12-active recurrence at the exact support level and makes the globally minimum-activity linear class more expensive at the value level;
5. no claim of a global rotor linear advantage is made until the relevant 14- and 15-active competitor classes are exhausted;
6. full-rotor differential value-level global optimization remains open; its support minimum is 13, while the best explicit DDT-compatible characteristics currently known use 14 active S-boxes and probability `2^-91` or `2^-92`.

The difference between static `2^-36` and the best current rotor 13-active value `2^-44.168297` is about 8.17 bits of individual-trail correlation cost, but this must not be described as an 8.17-bit security gain.

## 5. Reproduction

Compile the exact static all-maximum-DDT checker:

```bash
g++ -O3 -std=c++17 exact_static_diff_allmax_check.cpp -o exact_static_diff_allmax_check
```

Run the six required activity-12 shapes:

```bash
./exact_static_diff_allmax_check 2 6 4
./exact_static_diff_allmax_check 2 7 3
./exact_static_diff_allmax_check 2 8 2
./exact_static_diff_allmax_check 3 5 4
./exact_static_diff_allmax_check 3 6 3
./exact_static_diff_allmax_check 4 4 4
```

Compile the rotor boundary and exact minimum-activity LAT checkers:

```bash
g++ -O3 -std=c++17 rotor_linear_13_active_boundary_check.cpp -o rotor_linear_13_active_boundary_check
g++ -O3 -std=c++17 exact_rotor_linear_13_active.cpp -o exact_rotor_linear_13_active
```

Then run phases 0 through 3:

```bash
for r in 0 1 2 3; do
  ./rotor_linear_13_active_boundary_check "$r"
  ./exact_rotor_linear_13_active "$r"
done
```

Or use the verification driver:

```bash
python verify_three_round_value_checkpoint.py --compile-only
python verify_three_round_value_checkpoint.py --static
python verify_three_round_value_checkpoint.py --full
```

The full rotor verification is intentionally exhaustive and can be substantially slower than the earlier targeted probes.

## 6. Next exact target

The cleanest next target is rotor **phase 1**, because only 14-active linear trails can still beat its exact 13-active cost `44.168297` bits. If every 14-active phase-1 trail is excluded below that cost, `2^-44.168297` can be promoted to the exact global individual three-round linear-trail optimum for that phase.

Only after this higher-activity competition is handled should the same global value-level claim be considered for phases 0, 2, and 3 or extended to four rounds.
