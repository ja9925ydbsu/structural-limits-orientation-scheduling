# Mathematical Notes for Option 1

## 1. Elementary shear

Let the 16-byte state be `x = (x_0,...,x_15)` with each byte in `GF(2)^8`. For distinct byte indices `s` and `t`, define

`S_{t<-s,M}: x_t <- x_t XOR M x_s`,

with all other bytes unchanged and `M` any 8x8 binary matrix.

Because the source byte is unchanged by the operation,

`S_{t<-s,M}(S_{t<-s,M}(x))_t = x_t XOR M x_s XOR M x_s = x_t`.

Therefore every elementary shear is an involution. In particular, **M need not be invertible** for the shear itself to be invertible.

## 2. Composition

A finite composition of shears is invertible. If

`L = S_m o ... o S_2 o S_1`,

then

`L^{-1} = S_1 o S_2 o ... o S_m`,

because each `S_i^{-1} = S_i`. This gives an explicit inverse algorithm and proves that the full cross-byte layer is bijective.

## 3. Three-shear lifting cell

For a two-byte state `(a,b)`, use

1. `a <- a XOR A b`
2. `b <- b XOR B a`
3. `a <- a XOR C b`

where the second and third equations use the already-updated values. The resulting block map is

`[a']   [I+CB       A+C+CBA] [a]`
`[b'] = [B          I+BA    ] [b]`.

For the Option 1 baseline, `A=C=M_k` and `B=M_(k XOR 1)`. For all four `k`, each of the four displayed 8x8 blocks has rank 8 with the fixed development matrix family. Thus a one-active-byte input on either side creates two active output bytes: neither output dependency can vanish for a nonzero input byte.

This full-rank block property is stronger than required for invertibility of the lifting cell. It is imposed to make the cross-byte support expansion explicit and testable.

## 4. Butterfly support expansion

The 16 byte indices are paired in four stages with XOR masks 1, 2, 4, and 8. Starting from one active byte, each stage pairs every reached index with an index differing in a previously unused bit of the 4-bit byte index. Under the full-rank single-input dependency condition of the lifting cell, support therefore doubles at each stage:

`1 -> 2 -> 4 -> 8 -> 16`.

The reference self-test checks this statement exhaustively for all 16 byte positions and all 255 nonzero byte values, for both static and rotor schedules over the first four round indices.

## 5. Full linear rank

The 16-byte shear layer is a composition of invertible linear maps and must therefore have rank 128 over GF(2). An independent basis-image rank calculation confirms rank 128 for both static and rotor schedules in each tested round.

## 6. What remains unproved

The 1-to-16 support result is not a global byte-branch-number proof. Multi-active-byte inputs can create cancellations between injected terms. Determining the true minimum of

`wt_byte(x) + wt_byte(Lx)`

over all nonzero 128-bit inputs requires a separate exact search, MILP/SAT formulation, or equivalent proof. That is the next structural diagnostic before any security interpretation.
