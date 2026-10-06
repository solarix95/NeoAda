# 9. Dictionaries

A dictionary stores values by key rather than only by position. It is natural for small structured records and configuration data.

## Dictionary literals

```neoada
with Ada.Dict;
declare person : Dict := {"name": "Ada", "age": 42};
print(person{"name"});
```

## Defaults

The addon documents `value(key, default)` for optional data:

```neoada
print(person.value("country", "unknown"));
```

## Modeling data

Prefer stable, descriptive keys. Validate required keys when a dictionary crosses an important application boundary.

## Exercises

1. Model a book with title, author, and year.
2. Read a missing optional key with a default.
