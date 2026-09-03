# GitHub Publication Checklist

Repository destination:

`https://github.com/ja9925ydbsu/structural-limits-orientation-scheduling`

Before publishing the MDPI Cryptography submission snapshot:

1. Confirm that `paper/Structural_Limits_Orientation_Scheduling_MDPI.pdf` matches the current LaTeX source.
2. Confirm that `paper/Supplementary_Technical_Material_MDPI.pdf` matches the current supplementary source.
3. Keep the official MDPI LaTeX dependencies under `paper/Definitions/` so a clean clone can compile both documents.
4. Keep `LICENSE` at the repository root. Its MIT grant is explicitly limited to software under `src/` and `requirements.txt`.
5. Confirm that `results/structural_checks/revision8_additional_checks.json` remains present.
6. Run the primary structural scripts and both LaTeX builds from a clean clone.
7. Do not commit `__pycache__`, `.pyc`, LaTeX build debris, or local virtual environments.
8. If the manuscript receives a DOI or final journal citation, update `CITATION.cff` and `README.md`.

Suggested commit message:

`Prepare repository for MDPI Cryptography submission`

