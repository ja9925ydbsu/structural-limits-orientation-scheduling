# Reproducibility Guide

## Byte-local structural checks

Run from `src/byte_local/`:

```bash
python ddt_lat_null_checks.py --out ../../results/structural_checks/null_identity_checks.json
python routing_geometry_check.py --out ../../results/structural_checks/routing_geometry.json
```

The first script checks the AES S-box instance of the differential 8/255 identity and the linear Parseval analogue. The second checks the isolated routing cycle structure and four-round composition.

## 256-context transfer analysis

```bash
python weight1_transfer_256.py --keys 256 --transfer-keys 256 --out ../../results/weight1_256
```

The outputs include finite-round class probabilities, maximum starting states, best single trails, one-step retention, periodic transfer rates, paired schedule differences, and summary tables.

## Revision 8 structural audit

After the transfer outputs exist, run:

```bash
python revision8_structural_audit.py --results-root ../../results --out ../../results/structural_checks/revision8_additional_checks.json
```

This audit reports one-step minus periodic losses, paired sign consistency, the round-16 variance reduction, round-equivalent distance to the random-permutation benchmark, combined rotation-plus-routing transport orders, and the Perron-accessibility check for the maximum starting state.

## Matched empirical diagnostics

```bash
python matched_orientation_schedule_experiment.py --profile standard --out ../../results/matched_standard
```

These outputs are secondary diagnostics only.

## Cross-byte boundary study

The standard profile is run from `src/cross_byte/` with the supplied runner and modules. The wider-beam script is included for follow-up work, but no unfinished wider-beam result is used as manuscript evidence.

## Paper build

The manuscript and supplementary technical material use the official MDPI LaTeX class and supporting files under `paper/Definitions/`. From `paper/`, build the article and supplement with:

```bash
latexmk -pdf Structural_Limits_Orientation_Scheduling_MDPI.tex
latexmk -pdf Supplementary_Technical_Material_MDPI.tex
```

The repository also contains the compiled PDFs for review convenience. A full TeX Live installation supplies the standard packages required by the MDPI class.

