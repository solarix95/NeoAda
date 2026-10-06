# 25. A Complete Embedded Application

This chapter presents an architecture rather than inventing native-registration calls not yet documented in the public README.

## Application

Imagine a device simulator whose C++ core owns a temperature and an alarm output. NeoAda may read temperature and request alarm state, but not directly access files, network, or memory.

## Script contract

Conceptually:

```neoada
declare temperature : Number := Device:temperature();
if temperature > 80 then
    Device:setAlarm(true);
else
    Device:setAlarm(false);
end if;
```

The exact registration code must use the current `libneoada` API.

## Host responsibilities

C++ validates commands, owns lifetime, provides bounded execution, records failures, and defines a safe fallback.

## Integration testing

Test normal values, exact thresholds, invalid host state, syntax errors, unhandled exceptions, repeated execution, and object lifetime.

## Exercises

1. Add a second read-only sensor.
2. Define the safe fallback.
3. Write boundary tests around 80.
