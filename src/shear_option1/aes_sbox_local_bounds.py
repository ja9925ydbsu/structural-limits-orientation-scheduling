#!/usr/bin/env python3
"""Exact local differential and linear maxima for the AES S-box.

This is a small reproducibility aid for the Option 1 trail-bound notes.
"""
from __future__ import annotations


def gf_mul(a: int, b: int) -> int:
    out = 0
    for _ in range(8):
        if b & 1:
            out ^= a
        carry = a & 0x80
        a = (a << 1) & 0xFF
        if carry:
            a ^= 0x1B
        b >>= 1
    return out


def gf_pow(a: int, e: int) -> int:
    out = 1
    while e:
        if e & 1:
            out = gf_mul(out, a)
        a = gf_mul(a, a)
        e >>= 1
    return out


def rotl8(x: int, n: int) -> int:
    return ((x << n) | (x >> (8 - n))) & 0xFF


def aes_sbox(x: int) -> int:
    inv = 0 if x == 0 else gf_pow(x, 254)
    return inv ^ rotl8(inv, 1) ^ rotl8(inv, 2) ^ rotl8(inv, 3) ^ rotl8(inv, 4) ^ 0x63


def parity(x: int) -> int:
    return x.bit_count() & 1


def main() -> None:
    sbox = tuple(aes_sbox(x) for x in range(256))
    assert len(set(sbox)) == 256
    assert sbox[:16] == (
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,
        0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    )

    max_ddt = 0
    for dx in range(1, 256):
        counts = [0] * 256
        for x in range(256):
            counts[sbox[x] ^ sbox[x ^ dx]] += 1
        max_ddt = max(max_ddt, max(counts))

    max_walsh = 0
    for a in range(1, 256):
        for b in range(1, 256):
            walsh = 0
            for x in range(256):
                walsh += 1 if parity(a & x) == parity(b & sbox[x]) else -1
            max_walsh = max(max_walsh, abs(walsh))

    print(f"AES S-box maximum nonzero DDT entry: {max_ddt}/256 = 2^-6")
    print(f"AES S-box maximum nontrivial Walsh magnitude: {max_walsh}/256 = 2^-3")
    print(f"Maximum squared correlation: ({max_walsh}/256)^2 = 2^-6")


if __name__ == "__main__":
    main()
