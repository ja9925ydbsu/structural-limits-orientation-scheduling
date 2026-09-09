# Option 1 Cross-Byte Shear Development Track

This directory is an isolated development track for the cross-byte Hill-Enigma shear redesign. It does not modify the byte-local study or the existing manuscript checkpoint.

## Files

- `shear_core.py` — transparent reference primitives, coefficient derivation, rotor scheduling, the 32-shear directed-hypercube layer, and the experimental round/cipher.
- `shear_diagnostics.py` — exact first-pass checks for coefficient families, elementary and composed invertibility, 128-bit rank, one-active-byte support, static/rotor controls, and encryption/decryption round trips.
- `topology_search.py` — deterministic support-only hill-climb that reproduces the current direction mask `0x96699669`. The search objective does not use S-box probabilities, matrix coefficients, trail scores, or ciphertext statistics.
- `../../docs/OPTION1_CROSS_BYTE_SHEAR_SPEC.md` — formal v0.1 development specification and interpretation boundary.

## Run

From this directory:

```bash
python shear_diagnostics.py
python topology_search.py
```

The code uses only the Python standard library.

Expected first-pass diagnostics for the current candidate are:

- 32 coefficient seed families; all four orientations in every family are invertible and have bit branch number at least 4;
- elementary shears self-invert even with arbitrary or singular coefficient matrices;
- complete static and rotor shear layers have GF(2) rank 128;
- exact one-active-byte output support is minimum 5, mean 6.5, maximum 8;
- restricted one-active-byte diagnostic `B1 = 1 + 5 = 6`;
- 1-, 4-, and 16-round round-trip tests pass for both static and rotor schedules.

These are structural and correctness checks, not a security proof. In particular, `B1` is not the unrestricted byte branch number of the 128-bit linear layer.
