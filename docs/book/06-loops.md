# 6. Loops

Loops repeat work. NeoAda documents `for` and `while` loops together with `break`, `continue`, and Ada-style conditional forms.

## For loops

```neoada
for i in 1..10 loop
    print(i);
end loop;
```

## While loops

```neoada
declare remaining : Natural := 3;
while remaining > 0 loop
    print(remaining);
    remaining := remaining - 1;
end loop;
```

## Leaving or skipping iterations

Use early `break`/`continue` only when they make the rule clearer than the loop condition itself.

## Exercises

1. Print 1 through 5.
2. Write a countdown with a `while` loop.
