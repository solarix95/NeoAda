# 30. Complete Program: A Configuration Script

Embedded scripting can make configuration more expressive than static key/value files, but the host should expose only configuration capabilities.

## Prefer data over arbitrary power

Configuration scripts should compute or declare configuration, not silently perform unrelated I/O.

## Conceptual host API

```neoada
App:setTitle("Demo");
App:setRetryLimit(3);
App:enableFeature("logging", true);
```

These are application-defined, not built-in NeoAda functions.

## Validate twice

Reject invalid individual values at each API call, then validate the complete configuration before activation.

## Exercises

1. Design five configuration operations.
2. Define what happens when the script throws.
