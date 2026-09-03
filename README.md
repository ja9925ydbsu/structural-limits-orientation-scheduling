# Structural Limits of Orientation Scheduling in Byte-Local GF(2) Diffusion Layers

This repository supports the manuscript **Structural Limits of Orientation Scheduling in Byte-Local GF(2) Diffusion Layers** by Porter E. Coggins, III.

Repository: https://github.com/ja9925ydbsu/structural-limits-orientation-scheduling

## Manuscript status

The manuscript and supplementary technical material under `paper/` are formatted for submission to **Cryptography (MDPI)** using the official MDPI LaTeX class. The repository is the corresponding reproducibility companion and preserves the code, machine-readable results, and structural checks discussed in the paper.

## Why orientation scheduling was tested

Orientation scheduling is a controlled way to vary the linear coefficient pattern presented across rounds while holding the surrounding SPN fixed. For the admissible byte-local matrices used here, orientation preserves invertibility and the imposed branch-number floor while changing intra-byte coefficient placement. That makes it a plausible candidate for changing differential propagation, for example by disrupting repeatedly favourable S-box transitions, without replacing the S-box, changing state size, or redesigning the round function.

## Principal result and causal interpretation

The paper proves that changing the orientation of an independently applied byte-local 8 x 8 invertible GF(2) map can change coefficients and intra-byte difference patterns, but cannot enlarge active-byte support at that matrix step. The algebraic support statement is elementary. Its cryptanalytic role is causal: it identifies **active-byte support**, rather than coefficient diversity, as the structural variable that orientation cannot change.

The transfer analysis therefore measures the residual benefit available after diffusion width is held fixed. Weight-one recurrence is the minimum-support adversarial regime because exactly one byte and one S-box are active at each round boundary. Within the stated Markov-cipher model, the DDT mean-probability null is 8/255 for every bijective 8-bit S-box, and the same 8/255 null holds for mean squared correlation by Parseval. In the supplied AES-S-box harness, round-dependent scheduling produces a small periodic-rate separation because it changes temporal phase alignment. The one-step ordering reverses under the periodic operator, showing that the benefit is not improved support growth or uniformly better one-step transitions.

The structural theorem and the 8/255 nulls are not AES-specific. Measured finite and periodic rates are specific to the S-box, matrix contexts, and state-motion schedule and should be recomputed for other constructions.

## Repository layout

- `paper/` contains the MDPI Cryptography manuscript and supplementary sources and PDFs, bibliography, the required MDPI class and style files under `paper/Definitions/`, and Figure 1 source.
- `src/byte_local/` contains the matched four-arm experiment, 256-context transfer analysis, DDT/LAT identity checks, routing checker, and the retained Revision 8 structural-audit script used by Revision 9.
- `src/cross_byte/` contains the cross-byte Cauchy MDS boundary-study code and wider-beam follow-up runner.
- `results/weight1_256/` contains machine-readable transition and periodic-transfer results.
- `results/matched_standard/` contains secondary matched empirical diagnostics.
- `results/structural_checks/` contains independently recomputed null identities, routing geometry, phase decomposition, state-motion order, Perron-accessibility checks, statistical corrections, and MDS-gap checks.
- `docs/` contains scope, reproducibility, manuscript-to-code mapping, revision history, interpretation guidance, and repository upload guidance.

## Reproducing the primary checks

Create a Python environment and install the bundled requirements:

```bash
python -m venv .venv
# Windows: .venv\Scripts\activate
# macOS/Linux: source .venv/bin/activate
pip install -r requirements.txt
```

From `src/byte_local/`:

```bash
python ddt_lat_null_checks.py --out ../../results/structural_checks/null_identity_checks.json
python routing_geometry_check.py --out ../../results/structural_checks/routing_geometry.json
python weight1_transfer_256.py --keys 256 --transfer-keys 256 --out ../../results/weight1_256
python revision8_structural_audit.py --results-root ../../results --out ../../results/structural_checks/revision8_additional_checks.json
```

The matched empirical profile can be rerun with:

```bash
python matched_orientation_schedule_experiment.py --profile standard --out ../../results/matched_standard
```

The cross-byte study is a boundary setting only. Its pruned candidates remain far from the certified activity floor and are not used to rank schedules.

## Interpretation boundary

The 128-state transition computation is exact **within the Markov-cipher model stated in the manuscript**. DDT transition probabilities average over a uniform S-box input or independent uniform pre-S-box subkey. In the historical harness, round keys and seed matrices are deterministic functions of the same master key, so the model calculation is not a proof for a fixed keyed permutation.

The retained structural-audit script composes only whole-state rotation and routing for its transport-order calculation. It reports order 504 over four rounds and 60,060 over sixteen. Those values are not cipher periods because the byte-local matrix and S-box are intentionally omitted from that audit.

## Historical naming

The earlier experimental construction was called HESPN. That name remains only in a deterministic context label, a historical test vector, and compatibility filenames where changing the identifier would impair reproducibility. The public project name and manuscript framing describe the structural question rather than a cipher proposal.

## License scope

The MIT License in `LICENSE` applies only to software under `src/` and to `requirements.txt`. Manuscript text, figures, compiled papers, result data, and submission documents are not covered by that software license unless a separate license is stated later.

