# Option 1 topology comparison

This comparison belongs only to the `option1-cross-byte-shear` architectural-development branch. It is separate from the structural-limits-of-orientation-scheduling paper currently under review and should not be used to expand or revise that paper.

## Compared topologies

The same two-byte lifting cell and the same deterministic matrix family are used throughout. Only the network of byte-pair interactions changes.

- **chain:** sequential adjacent pairs `(0,1),(1,2),...,(14,15)`; 15 lifting cells; 45 shears.
- **ring:** the chain plus `(15,0)`; 16 lifting cells; 48 shears.
- **3-stage butterfly:** masks `1,2,4`; 24 lifting cells; 72 shears.
- **4-stage butterfly:** masks `1,2,4,8`; 32 lifting cells; 96 shears.

For the butterfly networks, the rotor schedule remains

`k = (round + stage + pair_index) mod 4`.

For chain and ring, each sequential edge is treated as a one-pair stage, so the same rule reduces to

`k = (round + stage) mod 4`.

The matched static control uses `k = 0`.

## Exact structural results

The program `exact_topology_comparison.cpp` constructs each complete 128-bit linear layer and determines its byte-level branch number by exhaustive GF(2) support/rank testing.

| topology | cells | shears | one-active-byte output support | byte-block rank range | exact `B_byte` | minimum witness split |
|---|---:|---:|---:|---:|---:|---:|
| chain | 15 | 45 | 2..16 | 0..8 | **3** | 1 -> 2 |
| ring | 16 | 48 | 3..16 | 0..8 | **4** | 1 -> 3 |
| 3-stage butterfly | 24 | 72 | 8..8 | 0..8 | **6** | 2 -> 4 |
| 4-stage butterfly | 32 | 96 | 16..16 | 8..8 | **8** | 4 -> 4 |

Every topology is full rank 128 because it is a composition of invertible shears; the exact computation confirms this for every tested schedule.

The table above is unchanged for:

- the static schedule; and
- rotor round phases 0, 1, 2, and 3.

Thus, for these four network choices and the present coefficient family, rotor scheduling does not change the coarse structural metrics in the table, including the exact byte-level branch number.

## Interpretation

The network topology is strongly visible in the diffusion metrics:

1. **Chain:** directional sequential processing permits a one-active-byte input to leave only two output bytes active in the weakest position, giving `B_byte = 3`.
2. **Ring:** closing the chain raises the weakest one-active-byte support from two to three and raises the branch number to 4, but the layer is still highly position dependent.
3. **3-stage butterfly:** every one-active-byte input reaches exactly eight output bytes, and the exact branch number rises to 6.
4. **4-stage butterfly:** every one-active-byte input reaches all 16 output bytes; all 256 byte-to-byte dependency blocks have rank 8; the exact branch number is 8.

The 4-stage butterfly therefore gives the strongest structural diffusion of these initial candidates, but it also has the largest operation count. The comparison should not be read as an efficiency or security ranking without an explicit cost model and later differential/linear analysis.

A particularly important negative result is that **rotor scheduling does not raise the exact branch number for any of these topologies**. The earlier static-versus-rotor diagnostic still shows coefficient-level differences for the 4-stage butterfly, including fewer branch-8 support-pair configurations under the rotor schedule, but those differences do not alter the branch-number optimum.

## Reproduction

From `src/shear_option1`:

```bash
g++ -O3 -std=c++17 exact_topology_comparison.cpp -o exact_topology_comparison

./exact_topology_comparison chain static 0
./exact_topology_comparison ring static 0
./exact_topology_comparison butterfly3 static 0
./exact_topology_comparison butterfly4 static 0

for r in 0 1 2 3; do
  ./exact_topology_comparison chain rotor "$r"
  ./exact_topology_comparison ring rotor "$r"
  ./exact_topology_comparison butterfly3 rotor "$r"
  ./exact_topology_comparison butterfly4 rotor "$r"
done
```

## Next questions

The current result justifies keeping the 4-stage butterfly as the reference architecture while exploring whether some of its 96-shear cost can be removed without collapsing the branch number. The next useful controls are therefore reduced-cost or cost-normalized variants, followed by coefficient-sensitive differential/linear trail diagnostics if the architecture continues to justify deeper study.
