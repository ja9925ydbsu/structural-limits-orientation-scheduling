#!/usr/bin/env python3
"""Verify exact global three-round linear closure for full rotor phases 2 and 3.

This belongs only to the Option 1 architectural-development branch.
It does not modify or support revisions to the structural-limits paper under review.
"""
from __future__ import annotations

import math
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
CHECKER = HERE / ".phase23_higher_active.verify.bin"
MIN13 = HERE / ".phase23_min13.verify.bin"

SPLITS14 = (
    (2,7,5),(2,8,4),(2,9,3),(2,10,2),
    (3,6,5),(3,7,4),(3,8,3),(3,9,2),
    (4,4,6),(4,5,5),(4,6,4),(4,7,3),(4,8,2),
    (5,4,5),(5,5,4),(5,6,3),(5,7,2),(6,4,4),
)

EXPECTED = {
    2: {"old13": 45.437758, "best14": 44.6828700736155, "split": (3,8,3)},
    3: {"old13": 45.460478, "best14": 44.6937647147188, "splits": {(2,8,4),(3,8,3)}},
}


def run(cmd: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    if check and p.returncode != 0:
        raise RuntimeError(f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}")
    return p


def compile_all() -> None:
    run(["g++","-O3","-std=c++17","-fopenmp",str(HERE/"exact_rotor_phase23_higher_active.cpp"),"-o",str(CHECKER)])
    run(["g++","-O3","-std=c++17",str(HERE/"exact_rotor_linear_13_active.cpp"),"-o",str(MIN13)])


def verify_phase(phase: int) -> None:
    exp = EXPECTED[phase]
    p13 = run([str(MIN13), str(phase)])
    m13 = re.search(r"cost_bits=([0-9.]+)", p13.stdout)
    if not m13 or not math.isclose(float(m13.group(1)), exp["old13"], abs_tol=5e-7):
        raise AssertionError(f"phase {phase}: 13-active value mismatch\n{p13.stdout}")

    found: dict[tuple[int,int,int], float] = {}
    discovery_target = exp["old13"] + 1e-6
    for split in SPLITS14:
        q = run([str(CHECKER), str(phase), "14", f"{discovery_target:.12f}", *map(str, split)], check=False)
        if q.returncode == 1:
            m = re.search(r"FOUND phase=\d+ total=14 best=([0-9.]+) split=(\d+)->(\d+)->(\d+)", q.stdout)
            if not m:
                raise AssertionError(q.stdout)
            found[tuple(map(int,m.group(2,3,4)))] = float(m.group(1))
        elif q.returncode != 0:
            raise RuntimeError(q.stderr or q.stdout)

    if not found:
        raise AssertionError(f"phase {phase}: expected a better 14-active trail")
    best = min(found.values())
    if not math.isclose(best, exp["best14"], abs_tol=5e-12):
        raise AssertionError(f"phase {phase}: expected {exp['best14']}, got {best}")

    # Closure pass: no 14-active split may be strictly below the recorded best.
    for split in SPLITS14:
        q = run([str(CHECKER), str(phase), "14", f"{exp['best14']:.15f}", *map(str, split)], check=False)
        if q.returncode == 1:
            raise AssertionError(f"phase {phase}: found 14-active trail below recorded optimum in {split}\n{q.stdout}")
        if q.returncode != 0:
            raise RuntimeError(q.stderr or q.stdout)

    if 15 * 3 <= exp["best14"]:
        raise AssertionError("15-active floor does not close higher activity")

    print(f"phase {phase}: exact global optimum = 2^-{exp['best14']:.13f}; 15-active floor = 45 bits")


def main() -> None:
    compile_all()
    verify_phase(2)
    verify_phase(3)


if __name__ == "__main__":
    main()
