# 3. Variables

A variable gives a value a name. Good variable names allow a program to read like a description of the problem it solves.

## Declaring and initializing

```neoada
declare score : Natural := 10;
declare title : String := "Round 1";
```

Prefer initializing variables close to where they are first needed.

## Assignment

```neoada
declare score : Natural := 10;
score := score + 5;
print(score);
```

## Choosing names

Prefer names that describe meaning: `remaining_attempts`, `file_name`, or `total`. Very short names are best kept for tiny local contexts.

## Exercises

1. Write declarations for width and height.
2. Update a numeric variable three times and print the final value.
