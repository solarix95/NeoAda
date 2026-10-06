# 26. Complete Program: Guess the Number

The public README documents output but not a complete interactive input API, so this chapter uses a deterministic teaching variant instead of inventing a function.

## Deterministic version

```neoada
declare secret : Natural := 7;
declare guess  : Natural := 5;

if guess = secret then
    print("Correct!");
elsif guess < secret then
    print("Too small.");
else
    print("Too large.");
end if;
```

## Lesson

Even this tiny game separates state, rules, and presentation. Larger programs benefit from the same separation.

## Exercises

1. Try guesses below, equal to, and above the secret.
2. Turn the comparison into a function.
