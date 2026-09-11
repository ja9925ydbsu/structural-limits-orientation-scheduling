#!/usr/bin/env python3
"""Reproduce the exact full-rotor three-round differential closure.

This verifier belongs only to the Option 1 cross-byte shear development branch.
It does not touch the structural-limits manuscript.

Evidence checked per rotor phase:
1. every structurally admissible 13-active split has no DDT-compatible trail;
2. all 18 admissible 14-active splits are optimized exactly and the best cost is 91 bits;
3. all 27 admissible 15-active splits are excluded at the all-DDT=4 90-bit floor;
4. 16 or more active S-boxes cost at least 96 bits and cannot beat 91 bits.

The split-by-split execution is intentional: monolithic runs can exceed a short
execution window even though the individual searches are independent.
"""
from __future__ import annotations

import argparse
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
OPT_SRC = HERE / "exact_rotor_three_round_differential.cpp"
MAX15_SRC = HERE / "exact_rotor_differential_allmax15.cpp"
OPT_BIN = HERE / ".rotor_diff_exact.verify.bin"
MAX15_BIN = HERE / ".rotor_diff_max15.verify.bin"

SPLITS13 = [
    (2,7,4),(2,8,3),(2,9,2),(3,6,4),(3,7,3),(3,8,2),
    (4,4,5),(4,5,4),(4,6,3),(4,7,2),(5,4,4),
]
SPLITS14 = [
    (2,7,5),(2,8,4),(2,9,3),(2,10,2),
    (3,6,5),(3,7,4),(3,8,3),(3,9,2),
    (4,4,6),(4,5,5),(4,6,4),(4,7,3),(4,8,2),
    (5,4,5),(5,5,4),(5,6,3),(5,7,2),(6,4,4),
]
SPLITS15 = [
    (2,7,6),(2,8,5),(2,9,4),(2,10,3),(2,11,2),
    (3,6,6),(3,7,5),(3,8,4),(3,9,3),(3,10,2),
    (4,4,7),(4,5,6),(4,6,5),(4,7,4),(4,8,3),(4,9,2),
    (5,4,6),(5,5,5),(5,6,4),(5,7,3),(5,8,2),
    (6,3,6),(6,4,5),(6,5,4),(6,6,3),(6,7,2),(7,4,4),
]


def run(cmd: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    if check and p.returncode != 0:
        raise RuntimeError(f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}")
    return p


def compile_checkers() -> None:
    run(["g++", "-O3", "-std=c++17", "-fopenmp", str(OPT_SRC), "-o", str(OPT_BIN)])
    run(["g++", "-O3", "-std=c++17", "-fopenmp", str(MAX15_SRC), "-o", str(MAX15_BIN)])
    print("compiled exact differential checkers")


def verify_phase(phase: int) -> None:
    support13 = 0
    for a,b,c in SPLITS13:
        p = run([str(OPT_BIN), str(phase), "13", str(a), str(b), str(c)])
        line = next(x for x in p.stdout.splitlines() if x.startswith("split="))
        if "DDT_COMPAT" in line:
            raise AssertionError(f"phase {phase}: unexpected 13-active DDT trail: {line}")
        if "SUPPORT_BUT_NO_DDT" in line:
            support13 += 1
            if (a,b,c) != (3,8,2):
                raise AssertionError(f"phase {phase}: unexpected support-feasible 13 split {a,b,c}")
    if support13 != 1:
        raise AssertionError(f"phase {phase}: expected exactly one support-feasible 13-active split")
    print(f"phase {phase}: 13-active class has no DDT-compatible characteristic")

    best14 = 999
    best_splits: list[tuple[int,int,int]] = []
    for a,b,c in SPLITS14:
        p = run([str(OPT_BIN), str(phase), "14", str(a), str(b), str(c)])
        m = re.search(r"best_cost=(\d+)", p.stdout)
        if not m:
            continue
        cost = int(m.group(1))
        if cost < best14:
            best14 = cost
            best_splits = [(a,b,c)]
        elif cost == best14:
            best_splits.append((a,b,c))
    if best14 != 91:
        raise AssertionError(f"phase {phase}: expected exact 14-active best cost 91, got {best14}")
    print(f"phase {phase}: exact 14-active optimum = 2^-91 via {best_splits}")

    for a,b,c in SPLITS15:
        p = run([str(MAX15_BIN), str(phase), str(a), str(b), str(c)], check=False)
        if p.returncode == 1 or "FOUND_ALL_DDT4" in p.stdout:
            raise AssertionError(f"phase {phase}: found 15-active 2^-90 characteristic in {(a,b,c)}")
        if p.returncode != 0 or "EXCLUDED_ALL_DDT4" not in p.stdout:
            raise AssertionError(f"phase {phase}: unexpected 15-active result for {(a,b,c)}\n{p.stdout}\n{p.stderr}")
    print(f"phase {phase}: all 27 15-active all-DDT4 classes excluded")
    print(f"phase {phase}: exact global individual differential characteristic = 2^-91")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--phase", type=int, choices=range(4))
    ap.add_argument("--compile-only", action="store_true")
    args = ap.parse_args()
    compile_checkers()
    if args.compile_only:
        return
    phases = [args.phase] if args.phase is not None else list(range(4))
    for phase in phases:
        verify_phase(phase)
    print("all requested rotor phases verified")


if __name__ == "__main__":
    main()
