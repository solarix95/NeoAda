# 8. Lists

Lists store ordered sequences. The `Ada.List` addon provides operations for inspecting and modifying list values.

## Loading helpers

```neoada
with Ada.List;
```

Documented methods include `length`, `isEmpty`, `append`, `extend`, `insert`, `first`, `last`, removal/take operations, search, sorting, and reversing.

## Mutation and copies

The API distinguishes in-place `sort()` from copy-returning `sorted()`, and similarly for reversing. Choose deliberately when you want to preserve the original.

## Searching carefully

The README describes `indexOf` as returning `Natural` while also using `-1` for “not found”. This needs reconciliation in the formal reference before production use.

## Exercises

1. Compare `append` and `extend`.
2. Sort a copy while preserving the original.
