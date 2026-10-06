# 19. Types as a Safety Tool

Types cannot prove that a program is correct, but they can make invalid states harder to express and accidental operations easier to detect.

## Make expectations visible

If a value is text, declare `String`. Use `Any` where dynamic behavior is intentional, not as a default.

## Validate at boundaries

External input may come from files, users, network data, or C++ callbacks. Validate it where it enters the trusted part of the program.

## Domain constraints

A type rarely expresses every application constraint. A percentage may need 0..100; a motor command may require a narrower range. Check such rules explicitly.

## High integrity is a system property

Strong typing helps, but high-integrity software also depends on requirements, tests, traceability, tools, deployment constraints, and the surrounding host system.

## Exercises

1. Identify three application constraints beyond basic type.
2. Replace an unnecessary `Any` with a precise type.
