#!/bin/bash

# Maximalzahl setzen
max=100000

# Schleife über jede Zahl von 2 bis max
for ((n=2; n<=max; n++)); do
    isPrime=1 # Annehmen, dass n eine Primzahl ist

    # Überprüfen, ob n durch irgendeine Zahl von 2 bis sqrt(n) teilbar ist
    for ((divisor=2; divisor*divisor<=n; divisor++)); do
        if (( n % divisor == 0 )); then
            isPrime=0 # n ist nicht prim
            break
        fi
    done

    # Wenn isPrime noch 1 ist, ist n eine Primzahl
    if (( isPrime == 1 )); then
        echo "$n"
    fi
done
echo
