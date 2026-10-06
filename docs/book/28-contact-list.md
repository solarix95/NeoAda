# 28. Complete Program: A Contact List

A contact list demonstrates dictionaries, lists, strings, and validation.

## One contact

```neoada
with Ada.Dict;
declare contact : Dict := {"name": "Ada", "city": "London"};
print(contact{"name"});
```

## Several contacts

A larger program can hold contact dictionaries in a list and use functions for search and formatting rather than manipulating raw structures everywhere.

## Validate

Decide mandatory fields, trim input, and reject invalid required values early.

## Exercises

1. Add an email field.
2. Design a validation function.
