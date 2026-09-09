#!/usr/bin/env python3
"""Enumerate partial fourth-stage masks for the Option 1 reduced-cost family.

The companion executable exact_reduced_cost_comparison performs each exact
GF(2) branch-number calculation. This wrapper groups all 256 final-stage masks
by retained-cell count and reports the best branch number found for a selected
schedule/round phase.
"""
from __future__ import annotations

import argparse
import re
import subprocess
from collections import defaultdict


def parse_result(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for token in text.strip().split():
        if "=" in token:
            key, value = token.split("=", 1)
            out[key] = value
    return out


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--binary",
        default="./exact_reduced_cost_comparison",
        help="path to compiled exact_reduced_cost_comparison executable",
    )
    parser.add_argument("--schedule", choices=("static", "rotor"), default="static")
    parser.add_argument("--round", type=int, default=0)
    args = parser.parse_args()

    best: dict[int, int] = defaultdict(lambda: -1)
    masks: dict[int, list[int]] = defaultdict(list)

    for mask in range(256):
        cmd = [args.binary, args.schedule, str(args.round), hex(mask)]
        result = subprocess.run(cmd, check=True, text=True, capture_output=True)
        fields = parse_result(result.stdout)
        branch = int(fields["byte_branch_number"])
        extra = mask.bit_count()
        if branch > best[extra]:
            best[extra] = branch
            masks[extra] = [mask]
        elif branch == best[extra]:
            masks[extra].append(mask)

    print(f"schedule={args.schedule} round={args.round}")
    for extra in range(9):
        total_cells = 24 + extra
        total_shears = 3 * total_cells
        representative = masks[extra][0] if masks[extra] else 0
        print(
            f"extra_final_cells={extra} total_cells={total_cells} "
            f"shears={total_shears} best_B={best[extra]} "
            f"best_mask_count={len(masks[extra])} "
            f"representative_mask=0x{representative:02x}"
        )


if __name__ == "__main__":
    main()
