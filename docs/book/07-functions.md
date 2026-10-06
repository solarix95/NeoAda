# 7. Procedures and Functions

Subprograms give operations a name. Procedures perform work; functions additionally return a typed result.

## Procedures

```neoada
procedure Show_Total(left, right : in Natural) is
begin
    print(left + right);
end Show_Total;
```

## Functions

```neoada
function Add(left, right : Natural) return Natural is
begin
    return left + right;
end Add;
```

## Parameter modes

Omitted or `in` parameters use value semantics. `out` is reference-backed and affects the caller. Current NeoAda `out` may also read the existing value, making it closer to Ada `in out` than write-only Ada `out`.

## Exercises

1. Write an `Add` function.
2. Explain the difference between `in` and current `out` behavior.
