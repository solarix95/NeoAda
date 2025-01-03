with Ada.List; -- list:length()

procedure bubbleSort(numbers : out list) is
begin
    declare n : Natural := numbers.length();
    declare swapped : Boolean := False;

    while true loop
        swapped := False;
       
        for i in 0..n-2 loop
            declare current : any := numbers[i];
            declare next    : any := numbers[i + 1];

            if current > next then
                -- swap current / next
                numbers[i] := next;
                numbers[i + 1] := current;

                swapped := True;
            end if;
        end loop;

        break when (swapped = false); 
    end loop;
end;

declare nums : list := [5, 2, 9, 42, 1, 5, 6];
print(nums);
bubbleSort(nums);
print(nums);

