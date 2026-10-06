# Author Notes — Verify Before Publishing

This draft avoids inventing NeoAda behavior where the public README is not precise. Check these points against source and unit tests before calling related examples normative.

## Language semantics

- Exact ranges/representation of `Natural` and `Number`.
- Overflow and underflow behavior.
- Complete Boolean operator syntax and short-circuit behavior.
- Full arithmetic/operator table and precedence.
- Exact list literal syntax and index base.
- Dictionary mutation syntax beyond documented reads.
- Scope, shadowing, recursion, and identifier case rules.

## Known documentation tension

The README documents `String.indexOf` and `List.indexOf` as returning `Natural`, while also saying not-found is `-1`. Reconcile these statements in the formal reference.

## Parameters

- Current `out` is readable/writable and close to Ada `in out`.
- Confirm literals/temporaries passed to `out`.
- Confirm COW detachment for mutation of `in` parameters.

## Exceptions

- Enumerate all built-in runtime exceptions.
- Document payload/message support if present.
- Confirm handler/`finally` failure precedence.

## Addons

- Synchronize every signature with implementation.
- Confirm slice/index bases and boundary behavior.
- Clarify whether string `length()` counts bytes, code points, or another unit.
- Confirm timezone/DST semantics.
- Confirm file-open failure exceptions and handle validity.

## Embedding API

The README shows `NeoAda::Runtime::execute(...)` and mentions `NdaState::unhandledException()`, but this book intentionally does not invent native registration calls. Add compilable registration examples after checking `libneoada` headers/examples.

## Documentation testing

Run every executable `neoada` code block in CI where possible. Mark conceptual examples explicitly so a documentation test harness can skip them.
