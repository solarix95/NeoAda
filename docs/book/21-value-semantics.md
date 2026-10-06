# 21. Value Semantics

NeoAda documents ordinary value behavior for inputs and copy-on-write storage for complex values such as `String`, `List`, `Dict`, and `Bytes`.

## Mental model

Passing a value as `in` gives the callee value semantics: local assignment or mutation should not change the caller's value.

## Copy-on-write

Complex values may initially share storage and detach only when one copy is modified. This preserves value semantics without eager deep copies.

## Writable parameters

`out` is reference-backed and caller-visible. Current behavior can read the old value too, so it resembles Ada `in out`.

## Why it matters

Predictability comes from knowing whether an operation can alter data already held elsewhere. Prefer interfaces where mutation is explicit.

## Exercises

1. Explain copy-on-write in plain language.
2. Contrast `in` and `out` for a `String`.
