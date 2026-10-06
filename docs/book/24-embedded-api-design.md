# 24. Designing an Embedded Scripting API

A good embedded API is intentionally smaller than the application behind it. Expose a stable problem-oriented interface, not every C++ detail.

## Keep the boundary small

Every exposed function is another behavior scripts can depend on. Start with the minimum useful capability.

## Validate inputs

Check types, ranges, string formats, resource identifiers, and state transitions at the host boundary.

## Query vs command

Where practical, separate operations that read state from operations that change state. Names should reveal side effects.

## Determinism and budgets

For high-integrity applications, control hidden time/randomness/global state and define execution, memory, recursion, I/O, and failure budgets.

## Exercises

1. Reduce a large C++ class to a five-operation script API.
2. Identify a nondeterministic dependency and propose an abstraction.
