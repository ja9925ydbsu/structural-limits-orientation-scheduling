# Reviewer Clarification Revision — 17 September 2026

Manuscript: **Structural Limits of Orientation Scheduling in Byte-Local GF(2) Diffusion Layers**

This revision addresses the reviewer’s follow-up comments on the reduced-round fixed-key calibration, terminology, deterministic-panel interpretation, and Figure 2 causal wording.

## Changes made

1. **Fixed-key rerun selection rule made explicit.** The manuscript and Supplement now state that the four 2^20 rerun cells were selected *post hoc*, after inspection of the initial 2^18 calibration, from zero-hit cells, with one selected cell from each schedule arm. They were higher-resolution discrepancy checks, not a prospectively specified, random, representative, or prevalence-estimating sample.

2. **Rerun cells identified in the Supplement.** A new table reports context, schedule, starting boundary bit, Markov probability, initial 2^18 observed count, 2^20 rerun count, and expected Markov count for all four cells. All four had zero observed weight-one outcomes in both the initial and rerun experiments despite model probabilities of order 10^-3.

3. **Model-bounded wording tightened.** “Cryptographically small” was replaced by “numerically small within the transfer model” or equivalent wording. The numerical schedule separation is not characterized as a fixed-key security advantage.

4. **Deterministic-panel interpretation harmonized.** The 256-context transfer panel is consistently treated as a reproducible descriptive panel rather than a formally sampled population. Confidence intervals retained in the Supplement apply only to separately sampled finite empirical screens, not to the deterministic transfer panel.

5. **Figure 2 and temporal-alignment wording audited.** Figure 2 remains explicitly conceptual rather than causal evidence. The manuscript, Supplement, and repository documentation now state that the one-step/periodic reversal is consistent with temporal-alignment effects but does not uniquely identify phase locking or any other mechanism.

6. **Repository synchronization.** The GitHub repository was synchronized with the revised manuscript, Supplement, reviewer-revision scripts, machine-readable calibration outputs, updated interpretation/reproducibility documentation, Figure 2 source, and compiled paper assets.
