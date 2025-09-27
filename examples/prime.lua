-- Primzahlprüfer von 2 bis 10000
local maximal = 100000
local zahl = 2

while zahl <= maximal do
    local ist_prim = true
    local teiler = 2

    while teiler <= zahl / 2 do
        if zahl % teiler == 0 then
            ist_prim = false
            break
        end
        teiler = teiler + 1
    end

    if ist_prim then
        print(zahl)
    end

    zahl = zahl + 1
end

