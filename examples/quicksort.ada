with Ada.List; -- list:length()

procedure quickSort(numbers : out list; low : Natural; high : Natural) is
begin
    if low < high then
        declare pivot_index : Natural := partition(numbers, low, high);
        quickSort(numbers, low, pivot_index - 1);  -- Rekursive Sortierung des linken Subarrays
        quickSort(numbers, pivot_index + 1, high); -- Rekursive Sortierung des rechten Subarrays
    end if;
end;

function partition(numbers : out list; low : Natural; high : Natural) return Natural is
begin
    declare pivot : any := numbers[high]; -- Pivot-Element
    declare i : Natural := low - 1;

    for j in low..high-1 loop
        if numbers[j] <= pivot then
            i := i + 1;
            -- swap numbers[i] and numbers[j]
            declare temp : any := numbers[i];
            numbers[i] := numbers[j];
            numbers[j] := temp;
        end if;
    end loop;

    -- swap numbers[i+1] and numbers[high] (swap pivot to the middle)
    declare temp : any := numbers[i + 1];
    numbers[i + 1] := numbers[high];
    numbers[high] := temp;

    return i + 1;
end;

declare nums : list := [5, 2, 9, 42, 1, 5, 6];
print(nums);
quickSort(nums, 0, nums.length() - 1);
print(nums);


