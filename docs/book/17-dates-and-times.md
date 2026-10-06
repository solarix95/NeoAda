# 17. Dates and Times

`Ada.DateTime` provides `Date`, `Time`, and `DateTime` objects for parsing, formatting, and simple arithmetic.

## Current time

```neoada
with Ada.DateTime;
declare now : DateTime := DateTime:now();
```

## Parsing and formatting

```neoada
declare start : DateTime := DateTime:fromString("2026-05-21 09:30:00", "yyyy-MM-dd HH:mm:ss");
print(start.toString("dd.MM.yyyy HH:mm"));
```

## Arithmetic

The addon includes `addDays` and `addSecs` as appropriate to the date/time type.

## Local time

For distributed or safety-relevant systems, define timezone and clock assumptions explicitly.

## Exercises

1. Parse and reformat a date-time.
2. Write down the timezone assumption for a logging application.
