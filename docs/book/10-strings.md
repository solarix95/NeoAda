# 10. Strings

`Ada.String` adds operations for text inspection, transformation, searching, formatting, and conversion to bytes.

## Searching

Documented methods include `length`, `contains`, `startsWith`, `endsWith`, and `indexOf`.

## In-place and copy forms

NeoAda distinguishes operations such as `upper()` from `toUpper()`, and `trim()` from `trimmed()`. The former mutate; the latter return a copy.

## Formatting

`String:format(value, format)` supports floating styles such as `f`/`e` and integer styles such as `d`, `x`, `b`, and `o`. Keep formatting at the presentation boundary.

## Exercises

1. Normalize user text by trimming and changing case.
2. Format a number for output.
