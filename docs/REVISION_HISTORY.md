# Revision History

## MDPI Cryptography submission format, 3 September 2026

- Recast the article and supplementary technical material in the official MDPI Cryptography LaTeX format.
- Added the MDPI class, bibliography styles, journal definitions, and logo assets required for a clean repository build.
- Added ORCID 0000-0003-3734-5478 to the article and supplementary author metadata.
- Omitted the historical known-answer compatibility values from the public supplement so they remain reserved for separate related work.
- Updated repository documentation, build instructions, citation metadata, and checksums for the MDPI submission snapshot.
- Preserved the research code, machine-readable results, figures, references, and substantive scientific content.


## Revision 9, 26 August 2026

- Added the explicit scientific motivation for testing orientation scheduling: it varies admissible coefficient placement and temporal presentation to later S-boxes while preserving byte locality, invertibility, the imposed branch-number floor, and the surrounding SPN.
- Clarified that Theorem 3.9 is algebraically elementary but is the principal causal result because it identifies active-byte support as the structural variable that orientation cannot change.
- Reframed the transfer result as the residual benefit of coefficient and phase variation after diffusion width is held fixed.
- Clarified that round-dependent scheduling is not inert: it can disrupt favourable temporal alignment even though it cannot create cross-byte support at the byte-local matrix step.
- Strengthened the conclusion that coefficient diversity and diffusion width are distinct design variables rather than substitutes.
- Added `docs/INTERPRETATION_GUIDE.md` and synchronized the editor letter, supplement, README, submission guidance, and manuscript copies.
- No experimental results, scripts, numeric tables, or machine-readable outputs were changed in Revision 9. The Revision 8 structural-audit script and its output filenames are retained for reproducibility.

## Revision 8, 26 August 2026

- Promoted the byte-local support result from proposition to **Theorem 3.9** and identified it as the principal result in the abstract, introduction, and conclusion.
- Strengthened the novelty claim by separating coefficient diversity from active-component support growth and linking the result explicitly to wide-trail intuition.
- Added discussion of which conclusions are S-box-independent and which measured rates remain AES-S-box and harness dependent.
- Defined the maximum in the weight-one probability table as the maximum over 128 possible starting weight-one boundary states.
- Added standard references for Markov ciphers, differential cryptanalysis, linear cryptanalysis, Parseval/correlation treatment, and Perron-Frobenius theory.
- Added one-step minus periodic rate decomposition and the one-step/periodic ordering reversal.
- Added a phase-locking interpretation that explains why temporally constant position-only scheduling groups with static scheduling.
- Restored the 256-context variance result, with about 42 percent lower average round-16 standard deviation for round-dependent schedules.
- Added descriptive sign consistency and a Perron-accessibility audit across all 1,024 schedule-context combinations.
- Quantified the schedule effect as approximately 0.076 of one round relative to the random-permutation benchmark under asymptotic continuation.
- Verified combined rotation-plus-routing transport-only orders of 504 over four rounds and 60,060 over sixteen, while keeping this distinct from the routing-only order-four theorem.
- Further compressed harness detail in the main article and moved normative implementation detail to the supplement.
- Clarified that empirical diagnostics cannot resolve the transfer-scale effect and are used only to exclude much larger differences.
- Corrected the acknowledgments wording so the work is described as an experimental design rather than a cipher design.
- Added a scoped MIT software license for `src/` and `requirements.txt` only.

## Revision 7

- Retitled the paper around the structural result.
- Elevated the byte-local support-growth proposition as the organizing contribution.
- Expanded the cryptanalytic motivation for weight-one recurrence.
- Promoted the aggregate-class versus best-single-trail gap as a transferable methodological observation.
- Moved detailed cross-byte beam-search and statistical diagnostics from the main article to the supplement.
- Renamed the public repository to `structural-limits-orientation-scheduling`.

## Revision 6

- Qualified “exact” by the Markov-cipher model.
- Stated the probability-averaging convention.
- Added the linear squared-correlation analogue.
- Proved the rotor versus round-only multiset identity and routing composition.
- Corrected statistical arithmetic and calibrated the cross-byte search against its MILP floor.
