# 16. Files

The `Ada.Io.File` family provides binary `File` and text-oriented `TextFile`. File access introduces resources that must be closed predictably.

## Opening files

The current API documents `open`, `openRead`, `create`, `append`, and `exists` variants for file objects.

## Text and binary operations

`TextFile` works with strings; binary `File` works with `Bytes`. Choose based on the data, not merely the filename.

## Guaranteed cleanup

```neoada
begin
    f := TextFile:openRead(path);
    return f.readAll();
finally
    f.close();
end;
```

Real code must also account for open failures and handle validity.

## Boundary validation

Paths and file contents are inputs. Embedded hosts should consider restricting which file operations scripts may access.

## Exercises

1. Read a text file.
2. Write a file and guarantee closure.
