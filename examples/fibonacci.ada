declare a: Natural := 0;  -- erste Fibonacci-Zahl
declare b: Natural := 1;  -- zweite Fibonacci-Zahl
declare temp: Natural;    -- temporäre Variable für den Austausch
declare count: Natural := 1;
declare max: Natural   := 93;  -- Anzahl der zu berechnenden Fibonacci-Zahlen

while count <= max loop
    print(a);
    temp := a + b;
    a := b;
    b := temp;
    count := count + 1;
end loop;

