# Appendix B — Built-in and Common Types

| Type | Purpose |
| --- | --- |
| `Natural` | whole-number/integer-like values |
| `Number` | general numeric values |
| `Boolean` | true/false conditions |
| `String` | text |
| `Any` | intentionally open runtime type |
| `List` | ordered container |
| `Dict` | key/value container |
| `Byte` | one byte |
| `Bytes` | binary buffer |

Addon-defined dictionary-backed objects include `File`, `TextFile`, `Date`, `Time`, and `DateTime`.

The project documents copy-on-write value semantics for `String`, `List`, `Dict`, and `Bytes`. A future formal reference should specify numeric ranges, overflow, literal grammar, index base, conversions, and sentinel behavior precisely.
