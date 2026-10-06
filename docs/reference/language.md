# Appendix A — NeoAda Syntax at a Glance

This summarizes syntax documented by the public NeoAda README. Interpreter and tests are authoritative.

## Variable
```neoada
declare x : Natural := 42;
```

## Conditional
```neoada
if x > 10 then
    print("greater");
elsif x = 10 then
    print("equal");
else
    print("less");
end if;
```

## Loops
```neoada
for i in 1..10 loop
    print(i);
end loop;

while x > 0 loop
    x := x - 1;
end loop;
```

## Function
```neoada
function Add(left, right : Natural) return Natural is
begin
    return left + right;
end Add;
```

## Exceptions
```neoada
begin
    raise MyError;
exception
    when MyError =>
        print("handled");
    when others =>
        raise;
finally
    -- cleanup
end;
```

## Addon and methods
```neoada
with Ada.String;
value.method();
Type:staticMethod();
```
