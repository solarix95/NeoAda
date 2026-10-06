# 27. Complete Program: A Small Text Adventure

A text adventure is useful for beginners because state is visible and rules can be added one at a time.

## Model state

```neoada
declare room : String := "hall";
if room = "hall" then
    print("You are standing in a quiet hall.");
elsif room = "garden" then
    print("You are outside in a garden.");
else
    print("You are somewhere unknown.");
end if;
```

## Grow with data

As the game expands, use dictionaries for room properties and lists for inventory. Keep command processing separate from the world model.

## Small increments

Build two rooms and one rule before designing fifty rooms. Small changes preserve understanding.

## Exercises

1. Add a third room.
2. Sketch an inventory list.
