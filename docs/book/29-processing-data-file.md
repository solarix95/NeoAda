# 29. Complete Program: Processing a Text Data File

Useful scripts often follow a pipeline: read, parse, validate, compute, format, write.

## Pipeline

Separate file reading, record parsing, validation, computation, and output. This makes malformed input easier to diagnose.

## Reading safely

Conceptually:

```neoada
with Ada.Io;
begin
    f := TextFile:openRead("input.txt");
    text := f.readAll();
finally
    f.close();
end;
```

Verify declaration and open-failure details against the current interpreter.

## Trust boundary

A file existing does not make its contents valid. Validate record shape, number formats, required values, and domain constraints.

## Exercises

1. Define a three-column text format.
2. List validation errors for one row.
