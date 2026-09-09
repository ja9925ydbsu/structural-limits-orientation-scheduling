#!/usr/bin/env python3
"""Option 1 cross-byte Hill-Enigma shear reference core.

Fresh architectural-development branch. This module intentionally does not inherit
HESPN v4's whole-state rotation or routing permutation. It isolates the proposed
cross-byte shear mechanism first, so those components can be added later only if
experiments justify them.

Research code only; not production cryptography.
"""
from __future__ import annotations

import hashlib
from dataclasses import dataclass
from functools import lru_cache
from typing import Iterable, Sequence

STATE_BYTES = 16
ROUNDS_DEFAULT = 16
MIN_BRANCH_NUMBER = 4
TOPOLOGY_MASK = 0x96699669  # 32 directed hypercube edges, stages d=0,1,2,3.

AES_SBOX = [
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
]
INV_SBOX = [0] * 256
for _i, _v in enumerate(AES_SBOX):
    INV_SBOX[_v] = _i

Matrix8 = tuple[int, ...]


@dataclass(frozen=True)
class ShearStep:
    stage: int
    edge_index: int
    source: int
    target: int


def byte_to_vec(x: int) -> list[int]:
    return [(x >> (7 - i)) & 1 for i in range(8)]


def vec_to_byte(v: Sequence[int]) -> int:
    out = 0
    for bit in v:
        out = (out << 1) | (bit & 1)
    return out


def apply_matrix_8(rows: Sequence[int], x: int) -> int:
    """Return Mx over GF(2), with rows stored as MSB-first bytes."""
    out = 0
    for row in rows:
        out = (out << 1) | ((row & x).bit_count() & 1)
    return out


def gf2_rank_8(rows: Sequence[int]) -> int:
    a = list(rows)
    rank = 0
    for col in range(8):
        bit = 1 << (7 - col)
        pivot = next((r for r in range(rank, 8) if a[r] & bit), None)
        if pivot is None:
            continue
        a[rank], a[pivot] = a[pivot], a[rank]
        for r in range(8):
            if r != rank and (a[r] & bit):
                a[r] ^= a[rank]
        rank += 1
    return rank


def is_invertible_8(rows: Sequence[int]) -> bool:
    return gf2_rank_8(rows) == 8


def branch_number_8(rows: Sequence[int]) -> int:
    return min(x.bit_count() + apply_matrix_8(rows, x).bit_count() for x in range(1, 256))


def branch_number_at_least(rows: Sequence[int], threshold: int) -> bool:
    for x in range(1, 256):
        wx = x.bit_count()
        if wx >= threshold:
            continue
        if wx + apply_matrix_8(rows, x).bit_count() < threshold:
            return False
    return True


def rows_to_grid(rows: Sequence[int]) -> list[list[int]]:
    return [byte_to_vec(row) for row in rows]


def grid_to_rows(grid: Sequence[Sequence[int]]) -> Matrix8:
    return tuple(vec_to_byte(row) for row in grid)


def rotate_matrix_entries_clockwise_90(rows: Sequence[int]) -> Matrix8:
    g = rows_to_grid(rows)
    rotated = [[0] * 8 for _ in range(8)]
    for i in range(8):
        for j in range(8):
            rotated[i][j] = g[7 - j][i]
    return grid_to_rows(rotated)


def rotate_matrix_entries_k(rows: Sequence[int], k: int) -> Matrix8:
    out = tuple(rows)
    for _ in range(k % 4):
        out = rotate_matrix_entries_clockwise_90(out)
    return out


def matrix_family(seed: Sequence[int]) -> tuple[Matrix8, Matrix8, Matrix8, Matrix8]:
    return tuple(rotate_matrix_entries_k(seed, k) for k in range(4))  # type: ignore[return-value]


def derive_round_key(master_key: bytes, round_index: int) -> bytes:
    return hashlib.sha256(
        master_key + b"SHEAR_ROUNDKEY" + round_index.to_bytes(2, "big")
    ).digest()[:16]


def derive_master_key_stub(password: str, salt: bytes) -> bytes:
    return hashlib.sha256(password.encode("utf-8") + salt).digest()


def derive_admissible_seed(master_key: bytes, edge_index: int,
                           min_branch: int = MIN_BRANCH_NUMBER) -> Matrix8:
    """Derive one edge coefficient seed by deterministic rejection sampling.

    The shear transformation itself is invertible for any 8x8 coefficient matrix.
    The legacy invertibility and branch-number filters are retained here only as a
    diffusion-quality baseline so their value can later be tested explicitly.
    """
    counter = 0
    while True:
        digest = hashlib.sha256(
            master_key + b"SHEAR_MATRIX" + edge_index.to_bytes(2, "big")
            + counter.to_bytes(4, "big")
        ).digest()
        rows = tuple(digest[:8])
        family = matrix_family(rows)
        if all(is_invertible_8(m) and branch_number_at_least(m, min_branch)
               for m in family):
            return rows
        counter += 1


@lru_cache(maxsize=16)
def coefficient_seeds(master_key: bytes, edge_count: int = 32,
                      min_branch: int = MIN_BRANCH_NUMBER) -> tuple[Matrix8, ...]:
    return tuple(derive_admissible_seed(master_key, e, min_branch) for e in range(edge_count))


def balanced_hypercube_steps(mask: int = TOPOLOGY_MASK) -> tuple[ShearStep, ...]:
    """Return the v0.1 32-shear, four-stage directed hypercube topology.

    Stage d connects byte positions that differ only in index bit d. Each stage
    has eight disjoint edges. One direction bit is stored per edge in `mask`:
    0 means low-index endpoint -> high-index endpoint; 1 means the reverse.
    """
    steps: list[ShearStep] = []
    edge_index = 0
    for stage in range(4):
        bit = 1 << stage
        for low in range(16):
            if low & bit:
                continue
            high = low | bit
            reverse = (mask >> edge_index) & 1
            source, target = (high, low) if reverse else (low, high)
            steps.append(ShearStep(stage, edge_index, source, target))
            edge_index += 1
    return tuple(steps)


BASE_STEPS = balanced_hypercube_steps()


def scheduled_matrix(seeds: Sequence[Matrix8], round_index: int, step: ShearStep,
                     rotor: bool = True) -> Matrix8:
    orientation = (round_index + step.edge_index) % 4 if rotor else 0
    return rotate_matrix_entries_k(seeds[step.edge_index], orientation)


def apply_shear_in_place(state: list[int], step: ShearStep, matrix: Sequence[int]) -> None:
    """x_target <- x_target XOR M x_source.

    This elementary shear is self-inverse when repeated with the same source,
    target, and M. No invertibility assumption on M is required.
    """
    state[step.target] ^= apply_matrix_8(matrix, state[step.source])


def apply_shear_layer(state: Sequence[int], master_key: bytes, round_index: int,
                      rotor: bool = True, steps: Sequence[ShearStep] = BASE_STEPS) -> list[int]:
    out = list(state)
    seeds = coefficient_seeds(master_key, max(step.edge_index for step in steps) + 1)
    for step in steps:
        apply_shear_in_place(out, step, scheduled_matrix(seeds, round_index, step, rotor))
    return out


def inverse_shear_layer(state: Sequence[int], master_key: bytes, round_index: int,
                        rotor: bool = True, steps: Sequence[ShearStep] = BASE_STEPS) -> list[int]:
    out = list(state)
    seeds = coefficient_seeds(master_key, max(step.edge_index for step in steps) + 1)
    for step in reversed(steps):
        apply_shear_in_place(out, step, scheduled_matrix(seeds, round_index, step, rotor))
    return out


def xor_bytes(a: Sequence[int], b: Sequence[int]) -> list[int]:
    return [x ^ y for x, y in zip(a, b)]


def round_function(block: bytes, master_key: bytes, round_index: int,
                   rotor: bool = True) -> bytes:
    """Experimental v0.1 round: AddRoundKey -> SubBytes -> CrossByteShear.

    Legacy whole-state rotation and routing are deliberately omitted at this stage.
    """
    if len(block) != STATE_BYTES:
        raise ValueError("block must contain exactly 16 bytes")
    keyed = xor_bytes(block, derive_round_key(master_key, round_index))
    subbed = [AES_SBOX[x] for x in keyed]
    return bytes(apply_shear_layer(subbed, master_key, round_index, rotor=rotor))


def inverse_round(block: bytes, master_key: bytes, round_index: int,
                  rotor: bool = True) -> bytes:
    unsheared = inverse_shear_layer(block, master_key, round_index, rotor=rotor)
    unsubbed = [INV_SBOX[x] for x in unsheared]
    return bytes(xor_bytes(unsubbed, derive_round_key(master_key, round_index)))


def encrypt_block(block: bytes, master_key: bytes, rounds: int = ROUNDS_DEFAULT,
                  rotor: bool = True) -> bytes:
    state = block
    for r in range(rounds):
        state = round_function(state, master_key, r, rotor=rotor)
    return state


def decrypt_block(block: bytes, master_key: bytes, rounds: int = ROUNDS_DEFAULT,
                  rotor: bool = True) -> bytes:
    state = block
    for r in reversed(range(rounds)):
        state = inverse_round(state, master_key, r, rotor=rotor)
    return state


def active_byte_count(state: Sequence[int]) -> int:
    return sum(value != 0 for value in state)


def pack_state_bits(state: Sequence[int]) -> int:
    return int.from_bytes(bytes(state), "big")


def gf2_rank_bitvectors(vectors: Iterable[int], width: int = 128) -> int:
    basis = [0] * width
    rank = 0
    for value in vectors:
        x = value
        while x:
            pivot = x.bit_length() - 1
            if basis[pivot]:
                x ^= basis[pivot]
            else:
                basis[pivot] = x
                rank += 1
                break
    return rank


def shear_layer_rank(master_key: bytes, round_index: int = 0, rotor: bool = True) -> int:
    images: list[int] = []
    for bit in range(128):
        state = bytearray(16)
        byte_index, bit_in_byte = divmod(bit, 8)
        state[byte_index] = 1 << (7 - bit_in_byte)
        images.append(pack_state_bits(apply_shear_layer(state, master_key, round_index, rotor)))
    return gf2_rank_bitvectors(images)
