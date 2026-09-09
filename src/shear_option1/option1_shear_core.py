#!/usr/bin/env python3
"""Option 1 cross-byte Hill-Enigma shear reference core.

Research prototype. The central primitive is a byte shear

    x_t <- x_t XOR M x_s

where M is an 8x8 binary matrix acting on one byte. The round-level
cross-byte layer is built only from such shears, so inversion is obtained by
replaying the same shears in reverse order.

This file intentionally does not inherit the historical HESPN routing or
whole-state rotation. It is a fresh architecture-development baseline.
"""
from __future__ import annotations

import hashlib
from dataclasses import dataclass
from typing import Sequence

STATE_BYTES = 16
ROUNDS_DEFAULT = 16

AES_SBOX = (
0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16,
)
INV_SBOX = [0] * 256
for _i, _v in enumerate(AES_SBOX):
    INV_SBOX[_v] = _i
INV_SBOX = tuple(INV_SBOX)

Matrix8 = tuple[int, ...]
IDENTITY8: Matrix8 = tuple(1 << (7-i) for i in range(8))

# Deterministically generated development seed:
# SHA256(b"OPTION1-HILL-SHEAR" || counter_be32), counter=1, first eight bytes.
# All four 90-degree orientations are distinct and rank 8.
BASE_MATRIX: Matrix8 = (0xC8, 0x73, 0xF5, 0x14, 0x01, 0xC0, 0xDE, 0x82)

def parity8(x: int) -> int:
    return (x & 0xFF).bit_count() & 1

def apply_matrix8(matrix: Matrix8, x: int) -> int:
    if len(matrix) != 8:
        raise ValueError("matrix must have eight rows")
    y = 0
    for i, row in enumerate(matrix):
        y |= parity8(row & x) << (7-i)
    return y

def rotate_matrix_clockwise(matrix: Matrix8) -> Matrix8:
    a = [[(matrix[r] >> (7-c)) & 1 for c in range(8)] for r in range(8)]
    b = [[a[7-c][r] for c in range(8)] for r in range(8)]
    return tuple(sum(b[r][c] << (7-c) for c in range(8)) for r in range(8))

def rotate_matrix_k(matrix: Matrix8, k: int) -> Matrix8:
    out = matrix
    for _ in range(k % 4):
        out = rotate_matrix_clockwise(out)
    return out

MATRIX_FAMILY: tuple[Matrix8, Matrix8, Matrix8, Matrix8] = tuple(
    rotate_matrix_k(BASE_MATRIX, k) for k in range(4)
)

def matrix_rank8(matrix: Matrix8) -> int:
    rows = list(matrix)
    rank = 0
    for bit in range(7, -1, -1):
        pivot = next((i for i in range(rank, 8) if (rows[i] >> bit) & 1), None)
        if pivot is None:
            continue
        rows[rank], rows[pivot] = rows[pivot], rows[rank]
        for i in range(8):
            if i != rank and ((rows[i] >> bit) & 1):
                rows[i] ^= rows[rank]
        rank += 1
    return rank

def compose8(a: Matrix8, b: Matrix8) -> Matrix8:
    columns = [apply_matrix8(a, apply_matrix8(b, 1 << (7-j))) for j in range(8)]
    rows = []
    for i in range(8):
        row = 0
        for j, col in enumerate(columns):
            if (col >> (7-i)) & 1:
                row |= 1 << (7-j)
        rows.append(row)
    return tuple(rows)

def xor_matrix8(a: Matrix8, b: Matrix8) -> Matrix8:
    return tuple(x ^ y for x, y in zip(a, b))

def shear(state: list[int], target: int, source: int, matrix: Matrix8) -> None:
    """x_target <- x_target XOR M*x_source. The operation is self-inverse."""
    state[target] ^= apply_matrix8(matrix, state[source])

@dataclass(frozen=True)
class ShearStep:
    target: int
    source: int
    orientation: int

def lifting_cell_steps(left: int, right: int, k: int) -> tuple[ShearStep, ShearStep, ShearStep]:
    """Three-shear two-byte cell with full-rank single-byte dependency blocks."""
    k &= 3
    return (
        ShearStep(left, right, k),
        ShearStep(right, left, k ^ 1),
        ShearStep(left, right, k),
    )

def butterfly_pairs(mask: int) -> tuple[tuple[int, int], ...]:
    if mask not in (1, 2, 4, 8):
        raise ValueError("mask must be one of 1,2,4,8")
    return tuple((i, i ^ mask) for i in range(16) if i < (i ^ mask))

def shear_layer_steps(round_index: int, rotor: bool = True) -> tuple[ShearStep, ...]:
    steps: list[ShearStep] = []
    for stage, mask in enumerate((1, 2, 4, 8)):
        for pair_index, (left, right) in enumerate(butterfly_pairs(mask)):
            k = (round_index + stage + pair_index) & 3 if rotor else 0
            steps.extend(lifting_cell_steps(left, right, k))
    return tuple(steps)

def apply_shear_steps(state: Sequence[int], steps: Sequence[ShearStep]) -> bytes:
    x = list(state)
    for step in steps:
        shear(x, step.target, step.source, MATRIX_FAMILY[step.orientation])
    return bytes(x)

def inverse_shear_steps(state: Sequence[int], steps: Sequence[ShearStep]) -> bytes:
    x = list(state)
    for step in reversed(steps):
        shear(x, step.target, step.source, MATRIX_FAMILY[step.orientation])
    return bytes(x)

def shear_layer(block: bytes, round_index: int, rotor: bool = True) -> bytes:
    return apply_shear_steps(block, shear_layer_steps(round_index, rotor))

def inverse_shear_layer(block: bytes, round_index: int, rotor: bool = True) -> bytes:
    return inverse_shear_steps(block, shear_layer_steps(round_index, rotor))

def derive_round_key(master_key: bytes, round_index: int) -> bytes:
    if len(master_key) != 32:
        raise ValueError("master_key must contain 32 bytes")
    h = hashlib.sha256()
    h.update(master_key)
    h.update(b"OPTION1-ROUNDKEY")
    h.update(round_index.to_bytes(2, "big"))
    return h.digest()[:16]

def round_forward(block: bytes, master_key: bytes, round_index: int, rotor: bool = True) -> bytes:
    if len(block) != 16:
        raise ValueError("block must contain 16 bytes")
    rk = derive_round_key(master_key, round_index)
    x = bytes(a ^ b for a, b in zip(block, rk))
    x = bytes(AES_SBOX[b] for b in x)
    return shear_layer(x, round_index, rotor)

def round_inverse(block: bytes, master_key: bytes, round_index: int, rotor: bool = True) -> bytes:
    rk = derive_round_key(master_key, round_index)
    x = inverse_shear_layer(block, round_index, rotor)
    x = bytes(INV_SBOX[b] for b in x)
    return bytes(a ^ b for a, b in zip(x, rk))

def encrypt_block(block: bytes, master_key: bytes, rounds: int = ROUNDS_DEFAULT, rotor: bool = True) -> bytes:
    x = block
    for r in range(rounds):
        x = round_forward(x, master_key, r, rotor)
    return x

def decrypt_block(block: bytes, master_key: bytes, rounds: int = ROUNDS_DEFAULT, rotor: bool = True) -> bytes:
    x = block
    for r in reversed(range(rounds)):
        x = round_inverse(x, master_key, r, rotor)
    return x

def lifting_cell_block_ranks(k: int) -> tuple[int, int, int, int]:
    a = MATRIX_FAMILY[k & 3]
    b = MATRIX_FAMILY[(k & 3) ^ 1]
    c = a
    cb = compose8(c, b)
    ba = compose8(b, a)
    cba = compose8(c, ba)
    p = xor_matrix8(IDENTITY8, cb)
    q = xor_matrix8(xor_matrix8(a, c), cba)
    r = b
    s = xor_matrix8(IDENTITY8, ba)
    return tuple(matrix_rank8(m) for m in (p, q, r, s))

def active_bytes(block: bytes) -> int:
    return sum(b != 0 for b in block)
