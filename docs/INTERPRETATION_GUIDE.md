# Interpretation Guide

## Why orientation scheduling was a reasonable hypothesis

The intervention changes coefficient placement across rounds while preserving the surrounding SPN, byte-local width, invertibility, and the imposed branch-number floor. It can therefore change which intra-byte difference patterns reach subsequent S-boxes and can change temporal alignment among favourable transitions. This makes orientation scheduling a natural candidate for improving differential diffusion without redesigning the rest of the round function.

## What Theorem 3.9 contributes

Theorem 3.9 is algebraically elementary: a byte-local map has no cross-byte coefficients, so it cannot directly activate another byte. Its importance is causal rather than algebraic. It identifies the structural variable that remains invariant under the scheduling intervention: active-byte support at the matrix layer.

This separates two design variables that can otherwise be conflated:

- **coefficient diversity**, which can change local transition probabilities, successor bit patterns, and temporal phase alignment; and
- **diffusion width**, which changes how many nonlinear components can become active.

The transfer analysis measures the residual schedule benefit after diffusion width has been held fixed. The observed small separation is therefore compatible with the theorem: scheduling is not inert, but its leverage is transition-level and temporal rather than support-growing.

## Design lesson

Public variation of a linear layer can be meaningful without being equivalent to wider diffusion. For byte-local maps, coefficient variation may disrupt favourable repeated transitions, but it cannot substitute for a mechanism that connects active byte supports. This is the causal interpretation used throughout Revision 9.
