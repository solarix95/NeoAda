# 5. Decisions

Programs become useful when they can choose between actions. NeoAda uses Ada-style `if`, `elsif`, `else`, and `end if`.

## A simple decision

```neoada
if temperature > 30 then
    print("It is hot.");
end if;
```

## Several alternatives

```neoada
if score > 90 then
    print("excellent");
elsif score > 70 then
    print("good");
else
    print("keep practicing");
end if;
```

## Readable conditions

Keep each condition focused. If a branch grows large, move its work into a procedure or function with a descriptive name.

## Exercises

1. Classify a score into three ranges.
2. Rewrite a nested decision as smaller named conditions.
