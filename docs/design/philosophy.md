# NeoAda Design Philosophy

This is a proposed articulation of NeoAda's design goals and should be reviewed against the maintainer's intent.

## Beginner-friendly does not mean vague

A learning language can be friendly while still teaching explicit concepts. Declared variables, visible types, named blocks, and readable control structures make program structure observable rather than magical.

## Readability over punctuation economy

Ada-inspired block syntax uses words such as `then`, `loop`, and explicit `end` forms. The goal is to let learners and reviewers see structure directly.

## Strong types as communication

Types state expectations. They help beginners form a correct mental model and experienced developers reason about boundaries.

## Value-oriented semantics

Copy-on-write containers aim to combine understandable value behavior with practical efficiency. Mutation should be intentional and visible.

## Small embedded surface

An embedded host should expose a problem-oriented API rather than its entire internal C++ object model. Small interfaces are easier to teach, test, secure, and keep compatible.

## Explicit failures

Named exceptions and `finally` support structured error handling. Programs should distinguish recoverable conditions from invariant violations.
