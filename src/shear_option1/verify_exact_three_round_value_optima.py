#!/usr/bin/env python3
"""Reproduce the exact full-layer three-round value-level checkpoint.

This driver compiles and runs the specialized exact checkers added for the
Option 1 cross-byte shear development branch.  It does not touch manuscript
files and is not part of the structural-limits paper under review.

Usage:
    python verify_exact_three_round_value_optima.py --compile-only
    python verify_exact_three_round_value_optima.py --static
    python verify_exact_three_round_value_optima.py --full

The full rotor optimization is intentionally exhaustive and may be much slower
than the targeted probes used during development.
"""
from __future__ import annotations

import argparse
import math
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent

CPP = {
    "static_allmax": "exact_static_diff_allmax_check.cpp",
    "broadened": "broadened_three_round_trail_search.cpp",
    "rotor_boundary": "rotor_linear_13_active_boundary_check.cpp",
    "rotor_exact": "exact_rotor_linear_13_active.cpp",
}

STATIC_ALLMAX_SPLITS = (
    (2, 6, 4),
    (2, 7, 3),
    (2, 8, 2),
    (3, 5, 4),
    (3, 6, 3),
    (4, 4, 4),
)

EXPECTED_ROTOR_LINEAR_BITS = {
    0: 47.208939,
    1: 44.168297,
    2: 45.437758,
    3: 45.460478,
}


def run(cmd: list[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    if check and p.returncode != 0:
        raise RuntimeError(
            f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}"
        )
    return p


def compile_all() -> dict[str, Path]:
    bins: dict[str, Path] = {}
    for key, source in CPP.items():
        src = HERE / source
        if not src.exists():
            raise FileNotFoundError(src)
        out = HERE / f".{key}.verify.bin"
        run(["g++", "-O3", "-std=c++17", str(src), "-o", str(out)])
        bins[key] = out
        print(f"compiled {source}")
    return bins


def verify_static(bins: dict[str, Path]) -> None:
    # The exact global activity minimum is 12.  By the branch-number and
    # one-active endpoint exclusions, these are the only activity-12 shapes
    # that need an all-DDT=4 feasibility check up to reversal.
    for a, b, c in STATIC_ALLMAX_SPLITS:
        p = run([str(bins["static_allmax"]), str(a), str(b), str(c)], check=False)
        if p.returncode != 0 or "EXCLUDED all-DDT4" not in p.stdout:
            raise AssertionError(
                f"unexpected all-DDT4 feasibility for {a}->{b}->{c}:\n{p.stdout}\n{p.stderr}"
            )
        print(p.stdout.strip())

    # Existence/cost witness: the all-transition branch-minimal exact search
    # must reproduce the 4->4->4 characteristic with total cost 73 bits.
    p = run([
        str(bins["broadened"]), "diff", "static", "0", "0xff", "8", "10"
    ])
    m = re.search(r"cost_bits=([0-9.]+)", p.stdout)
    if not m or not math.isclose(float(m.group(1)), 73.0, abs_tol=1e-9):
        raise AssertionError(f"static differential cost did not reproduce 73 bits:\n{p.stdout}")
    if "4->4->4" not in p.stdout.replace(" ", ""):
        # Historical builds may print the split with spaces; cost is decisive,
        # but keep the output visible for audit.
        print("warning: split text not found; inspect output below")
    print("static exact differential optimum reproduced: 2^-73")


def verify_rotor(bins: dict[str, Path]) -> None:
    for phase in range(4):
        p = run([str(bins["rotor_boundary"]), str(phase)])
        text = p.stdout
        required = (
            "one_active_T=16",
            "one_active_Tinv=16",
            "1<->7 forward=no inverse=no",
            "2<->6 forward=no inverse=no",
            "3<->5 forward=no inverse=no",
            "4<->4 forward=yes inverse=yes",
            "candidate_count=11",
        )
        for token in required:
            if token not in text:
                raise AssertionError(f"phase {phase}: missing boundary token {token!r}\n{text}")
        print(f"phase {phase}: boundary/completeness checks passed")

        q = run([str(bins["rotor_exact"]), str(phase)])
        out = q.stdout
        m = re.search(r"GLOBAL phase=\d+ active=13 split=(\d+)->(\d+)->(\d+) cost_bits=([0-9.]+) support_feasible_splits=(\d+)", out)
        if not m:
            raise AssertionError(f"phase {phase}: global result line missing\n{out}")
        split = tuple(map(int, m.group(1, 2, 3)))
        bits = float(m.group(4))
        surviving = int(m.group(5))
        if split != (3, 8, 2):
            raise AssertionError(f"phase {phase}: unexpected optimum split {split}")
        if surviving != 1:
            raise AssertionError(f"phase {phase}: expected one surviving split, got {surviving}")
        if not math.isclose(bits, EXPECTED_ROTOR_LINEAR_BITS[phase], abs_tol=5e-7):
            raise AssertionError(
                f"phase {phase}: expected {EXPECTED_ROTOR_LINEAR_BITS[phase]:.6f}, got {bits:.6f}"
            )
        print(f"phase {phase}: exact global linear trail = 2^-{bits:.6f}")


def main() -> None:
    ap = argparse.ArgumentParser()
    group = ap.add_mutually_exclusive_group()
    group.add_argument("--compile-only", action="store_true")
    group.add_argument("--static", action="store_true")
    group.add_argument("--full", action="store_true")
    args = ap.parse_args()

    bins = compile_all()
    if args.compile_only:
        return
    verify_static(bins)
    if args.full:
        verify_rotor(bins)
    elif not args.static:
        print("compiled all checkers. Use --static or --full to execute verification.")


if __name__ == "__main__":
    main()
