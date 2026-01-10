with Ada.String;

declare a: string := "Hello World";

procedure fprint(title: string; result: any) is
begin
 print(title & ": " & result);
end;

print("String examples:");

fprint("Original string",a);
fprint("length(method)", a.length());
fprint("length(operator)", #a);
fprint("toUpper",a.toUpper());

fprint("toLower", a.toLower());
fprint("indexOf 'Hello'", a.indexOf("Hello"));
fprint("indexOf 'World'", a.indexOf("World"));

a.append(" ");
fprint("length (untrimmed)",#a);
fprint("length (trimmed)",#a.trimmed());
fprint("slice",a.sliced(0,5));  -- Hello
fprint("slice",a.sliced(6,10)); -- World



