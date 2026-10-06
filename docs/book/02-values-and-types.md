# 2. Values and Types

Programs work with values: numbers, text, true/false decisions, collections, and other data. A **type** tells NeoAda what kind of value is expected and which operations make sense.

## Core types

The current documentation describes `Natural`, `Number`, `Boolean`, `String`, and `Any`. Container and addon chapters introduce `List`, `Dict`, `Bytes`, and dictionary-backed objects such as `Date`, `Time`, and `DateTime`.

## Why strong typing matters

```neoada
declare age : Natural := 12;
```

The declaration communicates intent to the interpreter and the reader. Strong typing helps detect accidental combinations before they spread through a program.

## The Any type

`Any` is useful when a value can legitimately have different runtime types. Use it deliberately rather than by default. The built-ins `typeof(value)` and `isany(value)` help inspect runtime values.

## Exercises

1. Declare variables for a name, age, and student flag.
2. Explain why `Any` should not automatically be used everywhere.
