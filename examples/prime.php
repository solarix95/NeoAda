<?php

$max = 100000; // Maximale Zahl

for ($n = 2; $n <= $max; $n++) {
    $isPrime = true; // Annahme, dass die Zahl n prim ist

    // Limit für die Prüfung auf Teiler setzt man auf die Wurzel von n
    for ($divisor = 2; $divisor < $n/2; $divisor++) {
        if ($n % $divisor === 0) {
            $isPrime = false; // n hat einen Teiler, also nicht prim
            break; // Abbrechen der inneren Schleife
        }
    }

    if ($isPrime) {
        echo $n . "\n";
    }
}

?>
