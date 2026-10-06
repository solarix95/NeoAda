# 13. Using Addons

NeoAda keeps optional functionality in addons. A script explicitly loads what it needs with `with`.

## The with statement

```neoada
with Ada.String;
with Ada.DateTime;
```

Explicit imports make dependencies visible.

## Current addon families

The public README documents strings, lists, dictionaries, bytes, text encoding, mathematics, file I/O, and date/time support.

## Naming

Type and method names are documented as case-insensitive, while examples use a preferred display style. Static methods use `Type:method(...)`; instance methods use `value.method(...)`.

## Exercises

1. List the addons needed for a script that reads text and manipulates strings.
