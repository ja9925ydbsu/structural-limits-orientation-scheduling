# Manuscript to Code Map

| Manuscript result | Code | Principal output |
| --- | --- | --- |
| Theorem 3.9 byte-support invariant and causal interpretation | analytic proof in `paper/Structural_Limits_Orientation_Scheduling_MDPI.tex`; no empirical code required | theorem and interpretation text |
| DDT 8/255 mean-probability null and averaging convention | `src/byte_local/ddt_lat_null_checks.py` | `results/structural_checks/null_identity_checks.json` |
| Linear squared-correlation analogue | `src/byte_local/ddt_lat_null_checks.py` | `results/structural_checks/null_identity_checks.json` |
| Four-round isolated routing composition | `src/byte_local/routing_geometry_check.py` | `results/structural_checks/routing_geometry.json` |
| 256-context weight-one iterative class | `src/byte_local/weight1_transfer_256.py` | `results/weight1_256/` |
| Periodic transfer rates | `src/byte_local/weight1_transfer_256.py` | `results/weight1_256/periodic_transfer_rates_by_context.csv` |
| Phase loss, sign consistency, variance, round-equivalent gap | `src/byte_local/revision8_structural_audit.py` | `results/structural_checks/revision8_additional_checks.json` |
| Combined rotation-plus-routing state-motion order | `src/byte_local/revision8_structural_audit.py` | `results/structural_checks/revision8_additional_checks.json` |
| Perron accessibility of maximizing start states | `src/byte_local/revision8_structural_audit.py` | `results/structural_checks/revision8_additional_checks.json` |
| Matched finite-resolution diagnostics | `src/byte_local/matched_orientation_schedule_experiment.py` | `results/matched_standard/` |
| Cross-byte boundary table | `src/cross_byte/run_mds_rotor_study.py` and modules | `results/mds_standard_profile_reported.csv` |
| Wider-beam follow-up | `src/cross_byte/mds_beam_sensitivity.py` | no convergence claim in current manuscript |

