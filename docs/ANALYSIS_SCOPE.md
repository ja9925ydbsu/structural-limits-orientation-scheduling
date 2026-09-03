# Analysis Scope and Claim Boundaries

## Principal theorem

Orientation scheduling was tested because it changes admissible coefficient placement and temporal presentation to later S-boxes while preserving byte locality, invertibility, the imposed branch-number floor, and the surrounding SPN. This makes it a plausible intervention for changing differential propagation without redesigning the round function.

The manuscript proves that an independently applied byte-local 8 x 8 linear map cannot enlarge active-byte support at that matrix step, regardless of matrix orientation. The algebraic statement is elementary, but its role is causal: it identifies the structural variable that the intervention cannot change. This separates coefficient diversity from diffusion width and defines how the measured scheduling effect should be interpreted.

The proof is independent of the AES S-box. The weight-one minimum-support interpretation also applies generally to bytewise SPNs in which one active byte corresponds to one active S-box.

## General one-step nulls

For every bijective 8-bit S-box, DDT column mass implies a mean weight-one differential retention of 8/255 under a uniform nonzero input difference. Parseval gives the same 8/255 mean squared-correlation retention for weight-one masks. These nulls are not AES-specific.

The measured finite and periodic rates are S-box- and harness-dependent because they depend on the actual transition distribution sampled by the scheduled matrices and on temporal phase ordering.

## Exact within a model

The weight-one transition enumeration and periodic transfer calculation are exact within the stated Markov-cipher DDT model. They remove transition-sampling error but do not remove the modelling assumption for a fixed keyed permutation.

## Phase-ordering interpretation

The retained structural-audit decomposition shows that static and position-only schedules lose rate between the phase-averaged one-step statistic and the periodic operator, while rotor and round-only remain near the one-step rate. The one-step ordering also flips under the periodic operator. This supports a phase-locking interpretation. It does not assert that one deterministic differential trail dominates the aggregate class.

## State-motion audit

The isolated routing composition has order four over a four-round block. When the actual whole-state rotations are composed with routing, while omitting the matrix and S-box, the transport-only permutation has order 504 over four rounds and 60,060 over sixteen. These are state-motion diagnostics, not cipher periods.

## Empirical and cross-byte material

Avalanche, collision, counter-distance, and other statistical screens are finite-resolution diagnostics only. The cross-byte boundary study changes width, field, and matrix family simultaneously, and its pruned candidates remain far from the certified activity floor. Neither is used as primary cryptanalytic evidence.
