from pathlib import Path
import base64
import io
import lzma
import tarfile

ROOT = Path(__file__).resolve().parents[1]
CHUNK_DIR = ROOT / ".github" / "revision_chunks_20260917"

b64 = "".join((CHUNK_DIR / f"chunk_{i:02d}.txt").read_text().strip() for i in range(6))
tar_bytes = lzma.decompress(base64.b64decode(b64))
with tarfile.open(fileobj=io.BytesIO(tar_bytes), mode="r:") as tf:
    tf.extractall(ROOT)

history = ROOT / "docs" / "REVISION_HISTORY.md"
entry = r"""## Reviewer clarification revision, 17 September 2026

- Stated explicitly that the four $2^{20}$ fixed-key rerun cells were selected post hoc after inspection of the initial $2^{18}$ calibration, with one zero-hit cell chosen from each schedule arm.
- Added the complete rerun-cell identifiers, Markov probabilities, initial counts, rerun counts, and expected surrogate counts to the Supplementary Material.
- Clarified that the selected reruns demonstrate persistent cellwise surrogate-to-harness discrepancies but do not estimate their prevalence across the full calibration grid.
- Replaced “cryptographically small” with “numerically small within the transfer model” or equivalent model-bounded wording.
- Made the deterministic 256-context panel consistently descriptive; confidence intervals remain only for separately sampled finite empirical screens.
- Retained Figure 2 as conceptual rather than causal evidence and removed uniquely causal phase-locking language from the manuscript, supplement, and repository documentation.
- Added the reviewer-revision calibration scripts and machine-readable outputs to the public repository.

"""
text = history.read_text()
if "## Reviewer clarification revision, 17 September 2026" not in text:
    if text.startswith("# Revision History\n"):
        text = "# Revision History\n\n" + entry + text[len("# Revision History\n"):].lstrip("\n")
    else:
        text = entry + text
    history.write_text(text)

print("Applied reviewer clarification revision payload.")
