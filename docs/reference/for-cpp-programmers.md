# Appendix G — NeoAda for C++ Programmers

## `Any` is not C++ `auto`
C++ `auto` performs compile-time type deduction. NeoAda `Any` is an open runtime type.

## Host-backed values are not C++ `volatile`
The README currently uses “volatile datatypes” for callback-backed values. Explain the behavior explicitly to avoid confusion with C++ `volatile`.

## Copy-on-write
`String`, `List`, `Dict`, and `Bytes` are documented with value semantics backed by shared storage that detaches on mutation.

## Embedding
The README shows `NeoAda::Runtime::execute(...)` and mentions `NdaState::unhandledException()`. Verify current headers and native registration API in `libneoada` for production code.
