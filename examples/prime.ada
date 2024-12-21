declare n: Natural;       -- Laufvariable von 2 bis 10000
declare divisor: Natural; -- möglicher Teiler
declare limit: Natural;   -- Obergrenze für Teilerprüfung (sqrt(n) approx.)
declare isPrime: Natural; -- 1 = prime, 0 = not prime
declare max: Natural;

max := 100000;

n := 2;
while n <= max loop
    -- print(n);
    -- Für n prüfen, ob sie prim ist
    if n = 2 then
        -- 2 ist per Definition prim
        print(n);
    else
        isPrime := 1;
        divisor := 2;

        -- limit ~ sqrt(n), hier vereinfacht als divisor*divisor <= n
        while (divisor <= n/2) and isPrime = 1 loop
        -- while (divisor * divisor) <= n and isPrime = 1 loop
            if (n mod divisor) = 0 then
                -- Wenn ein Teiler gefunden wird, ist n nicht prim
                isPrime := 0;
            else
                divisor := divisor + 1;
            end if;
        end loop;

        if isPrime = 1 then
            print(n);
        end if;
    end if;

    n := n + 1;
end loop;
