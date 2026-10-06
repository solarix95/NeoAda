# 12. Errors and Exceptions

NeoAda provides Ada-style exceptions, named handlers, `others`, re-raising, and `finally` cleanup.

## Raising and handling

```neoada
begin
    raise MyError;
exception
    when MyError =>
        print("handled MyError");
    when others =>
        print("handled something else");
end;
```

## Built-in exceptions

The README documents `ConstraintError` for arithmetic/value constraints and `ProgramError` for invalid program state.

## Re-raising

Inside a handler, `raise;` re-raises the current exception.

## Finally

A `finally` block runs before the protected block is left through normal exit, `return`, `break`, `continue`, or an unhandled exception. If `finally` raises, that new exception replaces the original one.

## Exercises

1. Raise and handle a named exception.
2. Describe guaranteed cleanup with `finally`.
