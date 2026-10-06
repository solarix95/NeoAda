# Appendix E — Exception Reference

## ConstraintError
Documented for arithmetic/value constraints such as division by zero and invalid addon inputs.

## ProgramError
Documented for invalid program state, including failed assignment to a strongly typed variable.

## Named exceptions and re-raise
Project examples show `raise MyError;` and `raise;` inside handlers.

## finally
The README documents that `finally` runs before control leaves the protected block. If `finally` raises, that exception replaces the original.

## C++
Unhandled script exceptions are documented as available through `NdaState::unhandledException()`.
