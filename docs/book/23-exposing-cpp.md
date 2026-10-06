# 23. Exposing C++ to NeoAda

NeoAda is designed to be extended from C++. Native functions and host-backed values should have narrow, explicit contracts.

## Native functions

Document accepted parameter types, return type, side effects, and failures. Validate script values before privileged host operations.

## Static and instance methods

Use static methods for constructor-like/global type operations and instance methods when behavior naturally belongs to one object.

## Host-backed values

The README calls callback-backed values “volatile datatypes”. Because C++ `volatile` means something else, teaching material should explain the behavior as host-backed or callback-backed values until terminology is finalized.

## Lifetime

A script object must not outlive the C++ object it references. Define ownership and invalid-handle behavior.

## Exercises

1. Write a contract for `set_speed(value)`.
2. List lifetime questions for a host object.
