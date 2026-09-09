#!/usr/bin/env python3
"""Exact first-pass diagnostics for the Option 1 cross-byte shear redesign."""
from __future__ import annotations

import json
import random
from pathlib import Path

from shear_core import (
    BASE_STEPS, MIN_BRANCH_NUMBER, TOPOLOGY_MASK, active_byte_count,
    apply_shear_in_place, apply_shear_layer, branch_number_8,
    coefficient_seeds, decrypt_block, derive_master_key_stub, encrypt_block,
    inverse_shear_layer, is_invertible_8, matrix_family, shear_layer_rank,
)

PASSWORD = "HillEnigmaSPN2026!"
SALT = bytes.fromhex("0102030405060708090A0B0C0D0E0F10")
PT = bytes.fromhex("00112233445566778899AABBCCDDEEFF")


def topology_reachability() -> list[dict[str, object]]:
    rows = []
    for start in range(16):
        active = {start}
        for step in BASE_STEPS:
            if step.source in active:
                active.add(step.target)
        rows.append({"start": start, "count": len(active), "positions": sorted(active)})
    return rows


def exact_weight1_support(key: bytes, round_index: int, rotor: bool) -> dict[str, object]:
    by_position = []
    all_counts = []
    for pos in range(16):
        counts = []
        for value in range(1, 256):
            state = [0] * 16
            state[pos] = value
            out = apply_shear_layer(state, key, round_index, rotor=rotor)
            counts.append(active_byte_count(out))
        by_position.append({
            "position": pos,
            "min": min(counts),
            "max": max(counts),
            "mean": sum(counts) / len(counts),
            "distinct": sorted(set(counts)),
        })
        all_counts.extend(counts)
    return {
        "round": round_index,
        "schedule": "rotor" if rotor else "static",
        "global_min": min(all_counts),
        "global_max": max(all_counts),
        "global_mean": sum(all_counts) / len(all_counts),
        "restricted_weight1_branch": 1 + min(all_counts),
        "by_position": by_position,
    }


def run() -> dict[str, object]:
    key = derive_master_key_stub(PASSWORD, SALT)
    seeds = coefficient_seeds(key)

    family_checks = []
    for e, seed in enumerate(seeds):
        fam = matrix_family(seed)
        bns = [branch_number_8(m) for m in fam]
        inv = [is_invertible_8(m) for m in fam]
        assert all(inv)
        assert min(bns) >= MIN_BRANCH_NUMBER
        family_checks.append({"edge": e, "invertible": inv, "branch_numbers": bns})

    # The elementary shear is self-inverse even with a singular coefficient matrix.
    rng = random.Random(20260909)
    singular = (0x00,) * 8
    step = BASE_STEPS[0]
    for _ in range(100):
        state = [rng.randrange(256) for _ in range(16)]
        original = list(state)
        apply_shear_in_place(state, step, singular)
        apply_shear_in_place(state, step, singular)
        assert state == original
        arbitrary = tuple(rng.randrange(256) for _ in range(8))
        state = list(original)
        apply_shear_in_place(state, step, arbitrary)
        apply_shear_in_place(state, step, arbitrary)
        assert state == original

    # Full layer inverse, independent of static/rotor schedule.
    for rotor in (False, True):
        for r in range(4):
            for _ in range(25):
                state = [rng.randrange(256) for _ in range(16)]
                mixed = apply_shear_layer(state, key, r, rotor=rotor)
                recovered = inverse_shear_layer(mixed, key, r, rotor=rotor)
                assert recovered == state

    # The composed linear layer must have full 128-bit rank.
    ranks = {}
    for rotor in (False, True):
        for r in range(4):
            rank = shear_layer_rank(key, r, rotor=rotor)
            assert rank == 128
            ranks[f"{'rotor' if rotor else 'static'}_r{r}"] = rank

    # Full round-trip for reduced and 16-round versions.
    ciphers = {}
    for rotor in (False, True):
        name = "rotor" if rotor else "static"
        for rounds in (1, 4, 16):
            ct = encrypt_block(PT, key, rounds=rounds, rotor=rotor)
            rt = decrypt_block(ct, key, rounds=rounds, rotor=rotor)
            assert rt == PT
            ciphers[f"{name}_{rounds}"] = ct.hex().upper()

    reach = topology_reachability()
    counts = [row["count"] for row in reach]
    assert min(counts) == 5 and max(counts) == 8 and sum(counts) == 104

    support = []
    for rotor in (False, True):
        for r in range(4):
            row = exact_weight1_support(key, r, rotor)
            assert row["global_min"] == 5
            assert row["global_max"] == 8
            assert row["restricted_weight1_branch"] == 6
            support.append(row)

    return {
        "project": "Option 1 cross-byte Hill-Enigma shear redesign",
        "topology_mask": f"0x{TOPOLOGY_MASK:08X}",
        "topology_steps": [step.__dict__ for step in BASE_STEPS],
        "temporal_reachability": reach,
        "coefficient_families": family_checks,
        "layer_ranks": ranks,
        "weight1_support": support,
        "ciphertexts": ciphers,
        "all_tests_passed": True,
    }


def main() -> None:
    result = run()
    out = Path(__file__).with_name("shear_diagnostics_results.json")
    out.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print("PASS: Option 1 shear diagnostics")
    print(f"Topology mask: {result['topology_mask']}")
    print("Temporal one-active support: min=5, mean=6.5, max=8")
    print("Restricted one-active-byte branch diagnostic: B1 = 6")
    print("All static/rotor layer ranks: 128")
    print(f"Wrote {out}")


if __name__ == "__main__":
    main()
