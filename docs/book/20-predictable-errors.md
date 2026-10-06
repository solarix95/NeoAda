# 20. Predictable Error Handling

An error strategy should answer: what can fail, who can recover, and what cleanup must always happen?

## Expected vs unexpected

A missing optional configuration value differs from division by zero or a broken invariant. Do not flatten all failures into one status if callers need different reactions.

## Catch where recovery is possible

A handler should normally retry, use a documented fallback, add context, or terminate a bounded operation safely. Otherwise propagation may be clearer.

## Cleanup

Keep `finally` blocks small and reliable because a failure from `finally` is documented to replace the original exception.

## Across C++

Unhandled script exceptions are documented as observable through `NdaState::unhandledException()`. The host should turn that into a defined application policy.

## Exercises

1. Classify three file-processing failures.
2. Define a host policy for an unhandled script exception.
