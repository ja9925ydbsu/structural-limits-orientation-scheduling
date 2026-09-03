#!/usr/bin/env python3
"""Structural slide- and reflection-symmetry audits for the MDS-rotor SPN.

These are diagnostics, not attack implementations.  They identify exact or
near-exact self-similarity that could motivate a dedicated attack: repeated
orientation rows, repeated full-round fingerprints, palindromic schedules,
and inverse-related matrix pairs across reflected rounds.
"""
from __future__ import annotations

import hashlib
import json
from typing import Sequence

from mds_rotor_core import (
    MDS_FAMILY,
    MDS_INVERSES,
    Schedule,
    derive_round_key,
    matrix_transpose,
    minimal_schedule_period,
    schedule_table,
)


def _matrix_hex(matrix) -> list[list[str]]:
    return [[f"{v:02X}" for v in row] for row in matrix]


def _round_fingerprint(master_key: bytes, schedule: Schedule, round_index: int,
                       include_key: bool) -> str:
    payload = bytearray()
    payload.extend(bytes(schedule(round_index, c) % 4 for c in range(4)))
    # ShiftRows and S-box are fixed; domain tags prevent ambiguity.
    payload.extend(b"AES-SBOX|SHIFTROWS|GF256-MDS")
    if include_key:
        payload.extend(derive_round_key(master_key, round_index))
    return hashlib.sha256(payload).hexdigest()


def slide_audit(master_key: bytes, schedule: Schedule, rounds: int) -> dict[str, object]:
    table = schedule_table(schedule, rounds)
    structural_fingerprints = [
        _round_fingerprint(master_key, schedule, r, include_key=False) for r in range(rounds)
    ]
    keyed_fingerprints = [
        _round_fingerprint(master_key, schedule, r, include_key=True) for r in range(rounds)
    ]

    repeated_structural: list[tuple[int, int]] = []
    repeated_keyed: list[tuple[int, int]] = []
    for a in range(rounds):
        for b in range(a + 1, rounds):
            if structural_fingerprints[a] == structural_fingerprints[b]:
                repeated_structural.append((a, b))
            if keyed_fingerprints[a] == keyed_fingerprints[b]:
                repeated_keyed.append((a, b))

    round_keys = [derive_round_key(master_key, r) for r in range(rounds)]
    key_hamming_distances = []
    for r in range(rounds - 1):
        key_hamming_distances.append(sum(
            (a ^ b).bit_count() for a, b in zip(round_keys[r], round_keys[r + 1])
        ))

    period = minimal_schedule_period(schedule, maximum=max(64, rounds * 2))
    return {
        "rounds": rounds,
        "orientation_table": table,
        "minimal_orientation_period": period,
        "repeated_structural_round_pairs": repeated_structural,
        "repeated_full_keyed_round_pairs": repeated_keyed,
        "all_round_keys_distinct": len(set(round_keys)) == len(round_keys),
        "adjacent_round_key_hamming_distances": key_hamming_distances,
        "minimum_adjacent_round_key_hamming_distance": min(key_hamming_distances) if key_hamming_distances else None,
        "assessment": (
            "Classical exact slide self-similarity is absent because no keyed round "
            "fingerprints repeat.  Structural schedule repetition remains visible and "
            "should be considered in advanced slide/related-key analysis."
            if not repeated_keyed else
            "Exact keyed round repetition detected; dedicated slide analysis is required."
        ),
    }


def reflection_audit(master_key: bytes, schedule: Schedule, rounds: int) -> dict[str, object]:
    table = schedule_table(schedule, rounds)
    palindromic_pairs = []
    inverse_matrix_pairs = []
    transpose_pairs = []
    key_equal_pairs = []
    key_xor_weights = []

    for r in range(rounds):
        s = rounds - 1 - r
        if r > s:
            break
        row_r = table[r]
        row_s = table[s]
        if row_r == row_s:
            palindromic_pairs.append((r, s))
        key_r = derive_round_key(master_key, r)
        key_s = derive_round_key(master_key, s)
        if key_r == key_s:
            key_equal_pairs.append((r, s))
        key_xor_weights.append({
            "round_pair": [r, s],
            "xor_hamming_weight": sum((a ^ b).bit_count() for a, b in zip(key_r, key_s)),
        })
        for col in range(4):
            o_r = row_r[col]
            o_s = row_s[col]
            if MDS_FAMILY[o_r] == MDS_INVERSES[o_s]:
                inverse_matrix_pairs.append((r, s, col, o_r, o_s))
            if MDS_FAMILY[o_r] == matrix_transpose(MDS_FAMILY[o_s]):
                transpose_pairs.append((r, s, col, o_r, o_s))

    exact_schedule_palindrome = table == list(reversed(table))
    return {
        "rounds": rounds,
        "orientation_table": table,
        "exact_schedule_palindrome": exact_schedule_palindrome,
        "palindromic_round_pairs": palindromic_pairs,
        "inverse_related_matrix_positions": inverse_matrix_pairs,
        "transpose_related_matrix_positions": transpose_pairs,
        "equal_reflected_round_keys": key_equal_pairs,
        "reflected_round_key_xor_weights": key_xor_weights,
        "fixed_round_order_is_self_inverse": False,
        "assessment": (
            "No exact encryption/decryption reflection was found: the round operation "
            "order is not self-inverse, reflected round keys are distinct, and no full "
            "inverse-matrix alignment spans the schedule.  Any partial palindromic or "
            "transpose relations are structural flags, not a demonstrated attack."
        ),
    }


def audit_all(master_key: bytes, schedules: dict[str, Schedule], rounds: int) -> dict[str, object]:
    return {
        name: {
            "slide": slide_audit(master_key, schedule, rounds),
            "reflection": reflection_audit(master_key, schedule, rounds),
        }
        for name, schedule in schedules.items()
    }


if __name__ == "__main__":
    from mds_rotor_core import SCHEDULES, derive_master_key
    print(json.dumps(audit_all(derive_master_key(), SCHEDULES, 16), indent=2))
