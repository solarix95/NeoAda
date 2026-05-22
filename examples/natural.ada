declare n: Natural := 42;


procedure main() is
begin
	print(n / 2);
	print(n mod 13);
	print(n * 2);
	print(n ** 2);
	print(n + 1);

	print(n / 0);

exception
	when ConstraintError => print("numeric error");

end;

main();

return 0;

