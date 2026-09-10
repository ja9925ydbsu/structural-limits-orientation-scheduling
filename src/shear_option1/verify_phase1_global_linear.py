#!/usr/bin/env python3
"""Verify the exact global three-round linear-trail optimum for rotor phase 1.

The globally minimum-activity 13-active class is checked by
exact_rotor_linear_13_active.cpp.  This driver then exhausts all 18 admissible
14-active weight splits with exact_rotor_phase1_14_active.cpp and verifies that
none beats the 13-active phase-1 cost.  Fifteen active S-boxes need no search:
the AES per-active-S-box correlation-cost floor is 3 bits, so 15 active S-boxes
cost at least 45 bits > 44.168296900785705.

Option 1 development branch only; not part of the structural-limits paper.
"""
from __future__ import annotations

import math
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
TARGET = 44.168296900785705
SPLITS = (
    (2, 7, 5), (2, 8, 4), (2, 9, 3), (2, 10, 2),
    (3, 6, 5), (3, 7, 4), (3, 8, 3), (3, 9, 2),
    (4, 4, 6), (4, 5, 5), (4, 6, 4), (4, 7, 3), (4, 8, 2),
    (5, 4, 5), (5, 5, 4), (5, 6, 3), (5, 7, 2),
    (6, 4, 4),
)


def run(cmd: list[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    if check and p.returncode != 0:
        raise RuntimeError(f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}")
    return p


def main() -> None:
    minact_src = HERE / "exact_rotor_linear_13_active.cpp"
    comp_src = HERE / "exact_rotor_phase1_14_active.cpp"
    minact_bin = HERE / ".phase1_13.verify.bin"
    comp_bin = HERE / ".phase1_14.verify.bin"

    run(["g++", "-O3", "-std=c++17", str(minact_src), "-o", str(minact_bin)])
    run(["g++", "-O3", "-std=c++17", "-fopenmp", str(comp_src), "-o", str(comp_bin)])

    p = run([str(minact_bin), "1"])
    m = re.search(r"GLOBAL phase=1 active=13 split=3->8->2 cost_bits=([0-9.]+)", p.stdout)
    if not m:
        raise AssertionError(f"phase-1 13-active result missing:\n{p.stdout}")
    rounded = float(m.group(1))
    if not math.isclose(rounded, TARGET, abs_tol=5e-7):
        raise AssertionError(f"unexpected 13-active cost {rounded}; expected about {TARGET}")
    print(f"phase 1: exact 13-active optimum reproduced: 2^-{TARGET:.12f}")

    for a, b, c in SPLITS:
        q = run([str(comp_bin), str(a), str(b), str(c)], check=False)
        if q.returncode != 0 or "no_trail_below_target" not in q.stdout:
            raise AssertionError(
                f"14-active competitor not excluded for {a}->{b}->{c}:\n{q.stdout}\n{q.stderr}"
            )
        print(f"excluded below target: {a}->{b}->{c}")

    if 15 * 3 <= TARGET:
        raise AssertionError("15-active local lower bound no longer exceeds target")

    print(f"15-active floor: 45 bits > {TARGET:.12f}")
    print(f"EXACT GLOBAL PHASE-1 LINEAR OPTIMUM: 2^-{TARGET:.12f}")


if __name__ == "__main__":
    main()
