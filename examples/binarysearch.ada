with Ada.List; -- list:length()

----------------------------------------------------------------------------------------------------
function binarySearch(numbers : list; target : any) return Natural is
begin
    declare low : Natural := 0;
    declare high : Natural := numbers.length() - 1;

    while low <= high loop
        declare mid : Natural := low + (high - low) / 2;
        declare mid_value : any := numbers[mid];

        if mid_value < target then
            low := mid + 1;
        elsif mid_value > target then
            high := mid - 1;
        else
            return mid; -- target found -> return index
        end if;
    end loop;

    return -1;         -- target not found
end;

----------------------------------------------------------------------------------------------------
declare nums : list := [1, 2, 5, 5, 6, 9, 42];
declare target : any := 5;
declare index : Natural := binarySearch(nums, target);

if index <> -1 then
    print("Element """ & target & """ has index: " & index);
else
    print("Element " & target & " not found.");
end if;

