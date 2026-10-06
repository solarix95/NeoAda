# 31. Complete Program: A Rule Engine Embedded in C++

Rule engines fit embedded scripting well: C++ owns trusted data/effects while scripts express changeable decisions.

## Facts, rules, effects

Separate host-supplied facts, NeoAda rule evaluation, and effects requested through a narrow API.

## Conceptual rule

```neoada
declare temperature : Number := Sensors:temperature();
-- Combine conditions using the Boolean syntax verified for your NeoAda version.
if temperature > 80 then
    Actions:requestShutdown();
end if;
```

## High-integrity considerations

Define rule ownership, review, versioning, deterministic inputs, budgets, logging, failure behavior, and independent safeguards. A script should not be the only barrier against a hazardous action unless the entire architecture/toolchain supports that assurance case.

## Boundary tests

For thresholds, test below, equal, and above. Test missing facts and exceptions. Record rule versions where traceability matters.

## Exercises

1. Design a rule with two inputs.
2. Write boundary tests.
3. Define which safety checks must remain in C++.
