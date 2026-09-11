#!/usr/bin/env python3
"""Reproduce the exact global three-round differential result for full rotor phases.

This script belongs only to the Option 1 cross-byte shear development branch.
It compiles the exact DDT join and all-DDT=4 checkers, then verifies:
  * all 13-active classes: only 3->8->2 is support-feasible and it is DDT-incompatible;
  * an explicit 91-bit 14-active characteristic exists in every phase;
  * no 14-active class attains the 90-bit all-DDT=4 floor;
  * no 15-active class attains the 90-bit all-DDT=4 floor;
  * 16 or more active S-boxes cost at least 96 bits.

The full run is exhaustive and can be substantially slower than targeted probes.
"""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent
VALUE_SRC = HERE / "exact_rotor_differential_value.cpp"
ALLMAX_SRC = HERE / "exact_rotor_differential_allmax.cpp"
VALUE_BIN = HERE / ".rotor_diff_value.verify.bin"
ALLMAX_BIN = HERE / ".rotor_diff_allmax.verify.bin"


def run(cmd: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
    p = subprocess.run(cmd, cwd=HERE, text=True, capture_output=True)
    if check and p.returncode != 0:
        raise RuntimeError(
            f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}\n{p.stderr}"
        )
    return p


def compile_checkers() -> None:
    for src, out in ((VALUE_SRC, VALUE_BIN), (ALLMAX_SRC, ALLMAX_BIN)):
        run(["g++", "-O3", "-std=c++17", "-fopenmp", str(src), "-o", str(out)])
        print(f"compiled {src.name}")


def candidates(total: int) -> list[tuple[int, int, int]]:
    out: list[tuple[int, int, int]] = []
    # One-active endpoints are excluded by the exact one-active support-16 result.
    for a in range(2, 8):
        for b in range(1, 13):
            c = total - a - b
            if not 2 <= c <= 7:
                continue
            if a + b < 8 or b + c < 8:
                continue
            # Exact branch-sum equality 8 occurs only at 4<->4.
            if a + b == 8 and (a, b) != (4, 4):
                continue
            if b + c == 8 and (b, c) != (4, 4):
                continue
            out.append((a, b, c))
    return out


WITNESS_SPLIT = {
    0: (2, 8, 4),
    1: (3, 8, 3),
    2: (3, 8, 3),
    3: (2, 8, 4),
}
EXPECTED_13_PAIRS = {0: 36, 1: 20, 2: 36, 3: 20}


def verify_phase(phase: int) -> None:
    c13 = candidates(13)
    assert len(c13) == 11
    for sp in c13:
        p = run([str(VALUE_BIN), str(phase), "13", *map(str, sp), "999"])
        line = p.stdout.splitlines()[0]
        if sp == (3, 8, 2):
            if "support=yes" not in line or "compatible=0" not in line:
                raise AssertionError(f"phase {phase}: expected support-only 3->8->2\n{p.stdout}")
            m = re.search(r"pairs=(\d+)", line)
            if not m or int(m.group(1)) != EXPECTED_13_PAIRS[phase]:
                raise AssertionError(f"phase {phase}: unexpected relation-pair count\n{p.stdout}")
        else:
            if "support=no" not in line:
                raise AssertionError(f"phase {phase}: unexpected 13-active support path {sp}\n{p.stdout}")
    print(f"phase {phase}: all 13-active characteristics excluded by exact support/DDT check")

    wsp = WITNESS_SPLIT[phase]
    p = run([str(VALUE_BIN), str(phase), "14", *map(str, wsp), "100"])
    if "best_bits=91" not in p.stdout or "compatible=0" in p.stdout:
        raise AssertionError(f"phase {phase}: 91-bit witness not reproduced\n{p.stdout}")
    print(f"phase {phase}: reproduced 14-active 2^-91 witness at {wsp[0]}->{wsp[1]}->{wsp[2]}")

    c14 = candidates(14)
    assert len(c14) == 18
    for sp in c14:
        q = run([str(ALLMAX_BIN), str(phase), "14", *map(str, sp)], check=False)
        if q.returncode != 0 or "all_DDT4=EXCLUDED" not in q.stdout:
            raise AssertionError(f"phase {phase}: 14-active 2^-90 candidate found at {sp}\n{q.stdout}")
    print(f"phase {phase}: all 18 14-active 2^-90 candidates excluded")

    c15 = candidates(15)
    assert len(c15) == 27
    for sp in c15:
        q = run([str(ALLMAX_BIN), str(phase), "15", *map(str, sp)], check=False)
        if q.returncode != 0 or "all_DDT4=EXCLUDED" not in q.stdout:
            raise AssertionError(f"phase {phase}: 15-active 2^-90 candidate found at {sp}\n{q.stdout}")
    print(f"phase {phase}: all 27 15-active 2^-90 candidates excluded")

    assert 16 * 6 == 96 and 96 > 91
    print(f"phase {phase}: exact global individual differential-characteristic optimum = 2^-91")


def main() -> None:
    compile_checkers()
    for phase in range(4):
        verify_phase(phase)
    print("ALL FULL-ROTOR PHASES VERIFIED: exact global three-round differential optimum 2^-91")


if __name__ == "__main__":
    main()
