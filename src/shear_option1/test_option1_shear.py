#!/usr/bin/env python3
"""Self-tests and first exact support diagnostics for Option 1."""
from __future__ import annotations

import random
from option1_shear_core import (
    BASE_MATRIX, MATRIX_FAMILY, active_bytes, apply_matrix8, decrypt_block,
    encrypt_block, inverse_shear_layer, lifting_cell_block_ranks, matrix_rank8,
    rotate_matrix_k, shear_layer,
)

def gf2_rank(vectors: list[int], width: int) -> int:
    rows = list(vectors)
    rank = 0
    for bit in range(width - 1, -1, -1):
        pivot = next((i for i in range(rank, len(rows)) if (rows[i] >> bit) & 1), None)
        if pivot is None:
            continue
        rows[rank], rows[pivot] = rows[pivot], rows[rank]
        for i in range(len(rows)):
            if i != rank and ((rows[i] >> bit) & 1):
                rows[i] ^= rows[rank]
        rank += 1
    return rank

def shear_layer_rank(round_index: int, rotor: bool) -> int:
    images = []
    for bit in range(128):
        x = bytearray(16)
        x[bit // 8] = 1 << (7 - (bit % 8))
        images.append(int.from_bytes(shear_layer(bytes(x), round_index, rotor), "big"))
    return gf2_rank(images, 128)

def main() -> None:
    assert len(set(MATRIX_FAMILY)) == 4
    assert MATRIX_FAMILY == tuple(rotate_matrix_k(BASE_MATRIX, k) for k in range(4))
    assert [matrix_rank8(m) for m in MATRIX_FAMILY] == [8, 8, 8, 8]

    for m in MATRIX_FAMILY:
        assert all(apply_matrix8(m, x) != 0 for x in range(1, 256))

    assert [lifting_cell_block_ranks(k) for k in range(4)] == [(8, 8, 8, 8)] * 4

    rng = random.Random(20260909)
    for rotor in (False, True):
        for r in range(8):
            for _ in range(200):
                x = bytes(rng.randrange(256) for _ in range(16))
                y = shear_layer(x, r, rotor)
                assert inverse_shear_layer(y, r, rotor) == x

    key = bytes(range(32))
    for rotor in (False, True):
        for rounds in (1, 2, 4, 8, 16):
            for _ in range(50):
                p = bytes(rng.randrange(256) for _ in range(16))
                c = encrypt_block(p, key, rounds, rotor)
                assert decrypt_block(c, key, rounds, rotor) == p

    for rotor in (False, True):
        for r in range(4):
            rank = shear_layer_rank(r, rotor)
            assert rank == 128
            counts = []
            for pos in range(16):
                for value in range(1, 256):
                    x = bytearray(16)
                    x[pos] = value
                    counts.append(active_bytes(shear_layer(bytes(x), r, rotor)))
            print(
                f"rotor={rotor} round={r}: rank={rank}; one-active-byte output support "
                f"min={min(counts)} max={max(counts)}"
            )
            assert min(counts) == 16 and max(counts) == 16

    print("PASS: matrix, lifting-cell, shear inversion, rank, cipher round-trip, and support checks")

if __name__ == "__main__":
    main()
