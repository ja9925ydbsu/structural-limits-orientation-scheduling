#!/usr/bin/env python3
"""Deterministic support-only search for 32-shear directed hypercube layouts.

The score uses temporal byte reachability only. It does not inspect S-boxes,
coefficient matrices, keys, trail probabilities, or empirical cipher outputs.
Accordingly, a high score is an architectural screening result, not a security claim.
"""
from __future__ import annotations

import random

SEED = 12345
RESTARTS = 400


def edges():
    out = []
    for d in range(4):
        bit = 1 << d
        out.extend((low, low | bit) for low in range(16) if not (low & bit))
    return out


EDGES = edges()


def support_counts(mask: int) -> tuple[int, ...]:
    counts = []
    for start in range(16):
        active = 1 << start
        for k, (low, high) in enumerate(EDGES):
            reverse = (mask >> k) & 1
            source, target = (high, low) if reverse else (low, high)
            if (active >> source) & 1:
                active |= 1 << target
        counts.append(active.bit_count())
    return tuple(counts)


def score(mask: int) -> tuple[int, int]:
    counts = support_counts(mask)
    return min(counts), sum(counts)


def hill_climb(mask: int, rng: random.Random) -> int:
    current = mask
    current_score = score(current)
    while True:
        candidates = list(range(32))
        rng.shuffle(candidates)
        improved = None
        for k in candidates:
            trial = current ^ (1 << k)
            trial_score = score(trial)
            if trial_score > current_score:
                improved = (trial, trial_score)
                break
        if improved is None:
            return current
        current, current_score = improved


def main() -> None:
    rng = random.Random(SEED)
    best = 0
    best_score = (-1, -1)
    for _ in range(RESTARTS):
        candidate = hill_climb(rng.getrandbits(32), rng)
        candidate_score = score(candidate)
        if candidate_score > best_score:
            best, best_score = candidate, candidate_score
    counts = support_counts(best)
    print(f"best mask = 0x{best:08X}")
    print(f"score = min {min(counts)}, total {sum(counts)}, mean {sum(counts)/16:.3f}, max {max(counts)}")
    print("counts =", counts)
    print("No global-optimality claim is made; this is deterministic hill-climb screening.")


if __name__ == "__main__":
    main()
