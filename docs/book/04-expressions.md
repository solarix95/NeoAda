# 4. Expressions

An expression computes a value. Expressions appear in assignments, conditions, function calls, and return statements.

## Arithmetic

```neoada
declare width  : Natural := 8;
declare height : Natural := 5;
declare area   : Natural := width * height;
```

## Comparisons

The README examples use `=` for equality and comparisons such as `>` inside conditions.

## String concatenation

NeoAda examples use `&` to combine strings:

```neoada
declare greeting : String := "Hello";
greeting := greeting & ", World";
```

## Clarity

When an expression mixes several operators, parentheses and intermediate variables can make the intended order obvious.

## Exercises

1. Compute a rectangle perimeter.
2. Build a greeting from two strings.
