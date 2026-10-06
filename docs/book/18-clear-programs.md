# 18. Writing Clear Programs

Readable code lowers the amount of hidden state a programmer must keep in mind and makes review, testing, and maintenance more reliable.

## Names matter

Choose names that describe purpose rather than implementation accidents.

## Keep subprograms focused

A function or procedure with one clear responsibility is easier to test and review than one that mixes parsing, state changes, I/O, and presentation.

## Prefer explicit code

Avoid hiding important conversions, units, mutation, or failure behavior behind vague names.

## Comments

Comment intent, assumptions, invariants, and surprising constraints rather than repeating obvious statements.

## Exercises

1. Rename vague variables in one of your scripts.
2. Split one long procedure into smaller ones.
