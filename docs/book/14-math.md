# 14. Numbers and Mathematics

`Ada.Math` provides mathematical constants and functions while keeping the core language small.

## Loading support

```neoada
with Ada.Math;
print(Math:pi());
print(Math:sqrt(9));
```

## Function groups

The addon documents rounding, roots/powers, logarithms, trigonometry, min/max/clamp, angle conversion, and checks for NaN/infinity/finite values.

## Numerical care

NaN and infinity can be valid floating-point results yet invalid application values. Validate numeric boundaries before crossing important interfaces.

## Units

Trigonometric functions are documented in radians. Make units visible in names or convert explicitly.

## Exercises

1. Compute a hypotenuse.
2. Check that a value is finite before using it.
