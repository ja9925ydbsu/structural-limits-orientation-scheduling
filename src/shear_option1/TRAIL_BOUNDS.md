# Option 1 conservative differential and linear trail bounds

**Scope boundary.** This file belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to revise or expand that paper.

This checkpoint converts the exact byte branch numbers already established for the Option 1 linear layers into conservative active-S-box and single-trail bounds. It does not claim resistance to differential or linear cryptanalysis, because differential and linear hull effects, key dependence, and value-level trail structure require additional analysis.

## Exact local AES S-box maxima

The script `aes_sbox_local_bounds.py` reconstructs the AES S-box algebraically and exhaustively evaluates its difference distribution table and linear approximation table.

It confirms:

- maximum nonzero DDT entry: `4/256 = 2^-6`;
- maximum nontrivial Walsh magnitude: `32/256 = 2^-3`;
- corresponding maximum squared correlation: `2^-6`.

Therefore, for a fixed differential characteristic containing `N` active AES S-boxes, the product of local transition probabilities is at most

`2^(-6N)`.

For a fixed linear trail containing `N` active AES S-boxes, the magnitude of the product correlation is at most

`2^(-3N)`,

and the squared-correlation bound is at most

`2^(-6N)`.

These are single-characteristic/single-trail bounds. They are not differential-hull or linear-hull bounds.

## Active-S-box bound from byte branch number

Each research round is

`AddRoundKey -> 16 parallel AES S-boxes -> linear shear layer`.

For two consecutive rounds, let `a` be the number of active S-boxes in the first round and `b` the number in the second. Because the S-box is bijective, nonzero byte differences remain nonzero through the S-box. The exact byte branch number of the intervening linear layer therefore gives

`a + b >= B_byte`.

For `R` rounds, grouping disjoint adjacent pairs gives the conservative lower bound

`N_active >= floor(R/2) * B_byte + (R mod 2)`.

The extra `+1` for an odd final round follows because a nonzero trail must still contain at least one active byte in that round.

## Current reference layers

The exact branch numbers established in the preceding checkpoints are:

- three-stage butterfly, 24 cells / 72 shears: `B_byte = 6`;
- selected reduced control `0x6f`, 30 cells / 90 shears: `B_byte = 7` for static and rotor phases 0 through 3;
- full four-stage butterfly, 32 cells / 96 shears: `B_byte = 8` for static and rotor phases 0 through 3.

For even round counts, the resulting conservative bounds are:

| layer | `B_byte` | 2 rounds: min active S-boxes | 4 rounds | 8 rounds | 16 rounds |
|---|---:|---:|---:|---:|---:|
| 3-stage butterfly | 6 | 6 | 12 | 24 | 48 |
| 30-cell `0x6f` control | 7 | 7 | 14 | 28 | 56 |
| full 4-stage butterfly | 8 | 8 | 16 | 32 | 64 |

Applying the AES local maxima gives the following single-differential-characteristic probability ceilings:

| layer | 2 rounds | 4 rounds | 8 rounds | 16 rounds |
|---|---:|---:|---:|---:|
| 3-stage butterfly | `2^-36` | `2^-72` | `2^-144` | `2^-288` |
| 30-cell `0x6f` control | `2^-42` | `2^-84` | `2^-168` | `2^-336` |
| full 4-stage butterfly | `2^-48` | `2^-96` | `2^-192` | `2^-384` |

The corresponding single-linear-trail correlation-magnitude ceilings are:

| layer | 2 rounds | 4 rounds | 8 rounds | 16 rounds |
|---|---:|---:|---:|---:|
| 3-stage butterfly | `2^-18` | `2^-36` | `2^-72` | `2^-144` |
| 30-cell `0x6f` control | `2^-21` | `2^-42` | `2^-84` | `2^-168` |
| full 4-stage butterfly | `2^-24` | `2^-48` | `2^-96` | `2^-192` |

Squared-correlation ceilings have the same exponents as the differential-characteristic table because the local squared-correlation maximum is also `2^-6` per active S-box.

## Interpretation limits

Several cautions are essential:

1. A branch-number argument counts active S-boxes but does not identify the best value-level characteristic.
2. Many characteristics can contribute to one differential; summing them can produce a differential probability larger than the best single-characteristic probability.
3. Many linear trails can contribute to one linear approximation; linear-hull effects can change the total correlation.
4. Exponents far below `2^-128` should not be presented as equivalent to security beyond a 128-bit block-size ceiling.
5. The current bounds do not address algebraic, integral, invariant-subspace, related-key, implementation, or side-channel attacks.

Accordingly, these tables are screening bounds for architecture comparison, not security claims.

## Schedule-sensitive 30-cell example

The fixed 30-cell mask `0x3f` remains a useful diagnostic because its exact branch number is 6 under the static schedule and 7 under each tested rotor phase. The two-round active-S-box lower bound is therefore 6 versus 7 for that exact same pruned topology.

This should still not be described as a general rotor advantage. The alternative fixed mask `0x6f` reaches branch number 7 under both static and rotor schedules, showing that cell placement and coefficient scheduling interact.

## Next analysis step

The next useful test is value-sensitive rather than activity-only: search for low-weight differential and linear trails using the actual AES DDT/LAT support together with the exact binary shear maps for the full 32-cell reference and the 30-cell `0x6f` control. Any such search should preserve the distinction among:

- theorem or exact finite proof;
- exhaustive computation;
- heuristic search result; and
- security interpretation.
