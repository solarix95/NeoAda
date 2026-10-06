# 11. Methods

Methods attach operations to a type-like receiver. NeoAda supports instance-style and static-style calls.

## Instance methods

```neoada
declare msg : String := "Hello!";
print(msg.length());
```

## Static methods

Static-style calls use a colon, for example `String:format(42.2, "f1")`.

## Method chaining

Current addon methods can be chained when each result supports the next operation. Break long chains into named values when each transformation deserves explanation.

## Exercises

1. Identify mutating vs copy-returning string methods.
2. Rewrite a long chain using intermediate variables.
