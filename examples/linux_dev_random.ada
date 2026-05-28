with Ada.Io.File;

declare f : File := File:openRead("/dev/random");

declare block : Any := f.read(4);

-- declare block : List  := f.read(10);

-- print(block[0]);

declare randomNumber : Natural;

if #block >= 4 then
	randomNumber := randomNumber + 
                       256_n**0 * block[0]  + 256_n**1 * block[1]  +
					   256_n**2 * block[2]  + 256_n**3 * block[3];
end if;

f.close();

return randomNumber;


