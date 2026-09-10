#!/usr/bin/env python3
"""Verify the exact global three-round linear-trail optimum for rotor phase 0.

Option 1 architectural-development branch only. This does not modify or
support expansion of the structural-limits-of-orientation-scheduling paper
under review.

The workflow is deliberately split by activity class:
  1. reproduce the exact 13-active phase-0 optimum;
  2. exhaust every admissible 14-active split and determine the best value;
  3. exhaust every admissible 15-active split below that 14-active value;
  4. use the AES 3-bit per-active-S-box floor to exclude 16+ active trails.

The higher-active checker is historically named
`exact_rotor_phase23_higher_active.cpp`, but it is parameterized by rotor phase
and is used here without changing its search space.
"""
from __future__ import annotations

import math
import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
MINACT_SRC = HERE / "exact_rotor_linear_13_active.cpp"
HIGHER_SRC = HERE / "exact_rotor_phase23_higher_active.cpp"
MINACT_BIN = HERE / ".phase0_minact.verify.bin"
HIGHER_BIN = HERE / ".phase0_higher.verify.bin"

EXPECTED_13 = 47.208939
EXPECTED_GLOBAL = 45.0156928096061
INITIAL_14_TARGET = 47.208940


def run(cmd: list[str], *, accept_one: bool = False) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    allowed = {0, 1} if accept_one else {0}
    if p.returncode not in allowed:
        raise RuntimeError(
            f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}"
        )
    return p


def compile_checkers() -> None:
    run(["g++", "-O3", "-std=c++17", str(MINACT_SRC), "-o", str(MINACT_BIN)])
    run([
        "g++", "-O3", "-std=c++17", "-fopenmp",
        str(HIGHER_SRC), "-o", str(HIGHER_BIN),
    ])


def splits(total: int) -> list[tuple[int, int, int]]:
    out: list[tuple[int, int, int]] = []
    for a in range(2, 8):
        for b in range(1, 13):
            c = total - a - b
            if c < 2 or c > 7:
                continue
            if a + b < 8 or b + c < 8:
                continue
            if a + b == 8 and (a, b) != (4, 4):
                continue
            if b + c == 8 and (b, c) != (4, 4):
                continue
            out.append((a, b, c))
    return out


def parse_found(text: str) -> float | None:
    m = re.search(r"FOUND phase=0 total=\d+ best=([0-9.]+)", text)
    return float(m.group(1)) if m else None


def main() -> None:
    compile_checkers()

    p = run([str(MINACT_BIN), "0"])
    m = re.search(r"GLOBAL phase=0 active=13 split=3->8->2 cost_bits=([0-9.]+)", p.stdout)
    if not m or not math.isclose(float(m.group(1)), EXPECTED_13, abs_tol=5e-7):
        raise AssertionError(f"13-active phase-0 value did not reproduce:\n{p.stdout}")
    print(f"13-active exact class optimum reproduced: 2^-{float(m.group(1)):.6f}")

    s14 = splits(14)
    if len(s14) != 18:
        raise AssertionError(f"expected 18 admissible 14-active splits, got {len(s14)}")
    best14 = math.inf
    best14_split: tuple[int, int, int] | None = None
    for a, b, c in s14:
        q = run([
            str(HIGHER_BIN), "0", "14", f"{INITIAL_14_TARGET:.12f}",
            str(a), str(b), str(c),
        ], accept_one=True)
        v = parse_found(q.stdout)
        if v is not None and v < best14:
            best14 = v
            best14_split = (a, b, c)
        print(q.stdout.strip())

    if best14_split != (3, 8, 3):
        raise AssertionError(f"unexpected best 14-active split: {best14_split}")
    if not math.isclose(best14, EXPECTED_GLOBAL, abs_tol=5e-10):
        raise AssertionError(f"unexpected best 14-active cost: {best14}")
    print(f"best 14-active phase-0 trail: 2^-{best14:.13f} via 3->8->3")

    s15 = splits(15)
    if len(s15) != 27:
        raise AssertionError(f"expected 27 admissible 15-active splits, got {len(s15)}")
    for a, b, c in s15:
        q = run([
            str(HIGHER_BIN), "0", "15", f"{best14:.15f}",
            str(a), str(b), str(c),
        ], accept_one=True)
        if q.returncode != 0 or "EXCLUDED phase=0 total=15" not in q.stdout:
            raise AssertionError(
                f"15-active competitor below the phase-0 candidate for {a}->{b}->{c}:\n{q.stdout}"
            )
        print(q.stdout.strip())

    if 16 * 3 <= best14:
        raise AssertionError("16-active AES correlation-cost floor does not close the search")

    print(
        "PASS: exact global full-rotor phase-0 three-round individual linear-trail "
        f"magnitude = 2^-{best14:.13f}"
    )


if __name__ == "__main__":
    main()
