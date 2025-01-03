
----------------------------------------------------------------------------------------------------
function factorial(n : Natural) return Natural is
begin
    if n <= 0 then
        return 1;                    -- default: factorial of 0 is 1
    else
        return n * factorial(n - 1); -- recursion: n * (n-1)!
    end if;
end;

----------------------------------------------------------------------------------------------------
declare num    : Natural := 7;
declare result : Natural := factorial(num);

print("Factorial of " & num & " is " & result);

