# 1. Getting Started

A program is a sequence of instructions. NeoAda is designed so that many of those instructions read almost like carefully written English. In this chapter you will run a first program, learn how statements end, and see how comments make code easier to understand.

## Your first NeoAda program

The traditional first program prints a message:

```neoada
declare msg : String := "Hello, world!";
print(msg);
```

The first line declares a variable named `msg`. The second line calls the built-in `print` function. NeoAda statements normally end with a semicolon.

## Comments and formatting

A comment begins with two hyphens:

```neoada
-- This line explains the next statement.
print("Hello");
```

Indent nested statements consistently. Explicit endings such as `end if;` and `end loop;` make structure visible.

## Learning from errors

Error messages are part of programming. Identify the line, the operation, and the type or construct involved. Change one thing at a time, run again, and observe.

## Exercises

1. Change the greeting so that it includes your name.
2. Create two `String` variables and print both.
3. Remove a semicolon and inspect the resulting error.
