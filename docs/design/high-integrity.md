# NeoAda and High-Integrity Software

NeoAda can be designed **with high-integrity and safety-conscious software development in mind** without implying that the language, runtime, interpreter, or toolchain is certified for a safety standard.

## Helpful properties

Potentially useful properties include explicit declarations and strong typing, visibly terminated control structures, value semantics for ordinary inputs, visible writable parameters, structured exceptions, explicit addon loading, and narrow C++ embedding interfaces.

These properties can reduce ambiguity, but they do not by themselves establish system safety.

## Responsibilities outside the language

A high-integrity deployment may also require precise requirements and hazard analysis, deterministic execution and resource bounds, verified numeric behavior, tool confidence, traceable tests/coverage, configuration control, independent safeguards, controlled I/O, and auditable fallback behavior.

## Recommended wording

Until supported by evidence for a specific standard and scope, prefer:

> NeoAda is designed with high-integrity and safety-conscious software development in mind.

Avoid “certified for safety-critical systems” unless the exact certification scope, version, target, toolchain, and standard are documented.

## Useful roadmap

Document and test integer/floating semantics, overflow/conversions, memory/resource limits, determinism and callbacks, exception propagation, container/index boundaries, stack/recursion behavior, execution limits, parser/runtime fuzzing, and release compatibility guarantees.
