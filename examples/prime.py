maximal = 100000
zahl = 2

while zahl <= maximal:
    ist_prim = True
    teiler = 2

    while teiler <= zahl // 2:
        if zahl % teiler == 0:
            ist_prim = False
            break
        teiler += 1

    if ist_prim:
        print(zahl)

    zahl += 1
