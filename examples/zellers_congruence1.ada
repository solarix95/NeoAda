
---------------------------------------------------------------------------------------------------
procedure Day_Of (Day, Month, Year : in Natural;
                      Result       : out Natural) is
        M : Natural := Month;
        Y : Natural := Year;
        C : Natural;
begin
  if M < 3 then
    Y := Y - 1;
    M := M + 10;
  else
    M := M - 2;
   end if;

   C      := Y / 100;          -- first two digits of Year
   Y      := Y mod 100;        -- last two digits of Year
   Result := ((26*M - 2)/10 + Day + Y + Y/4 + C/4 - 2*C) mod 7;
end Day_Of;

---------------------------------------------------------------------------------------------------
declare R : Natural;

Day_Of(25, 1, 1956, R);
print(R);


