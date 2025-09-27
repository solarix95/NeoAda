import std.stdio;  // Für die Ausgabe (writeln)
import std.math;   // Für sqrt (optional, wenn du divisor*divisor <= n verwendest, brauchst du kein sqrt)
 
void main()
{
    // Wir beginnen bei 2 (die erste Primzahl) und gehen bis 10000
    for(int n = 2; n <= 100000; n++)
    {
        bool isPrime = true;

        // Sonderfall 2 (ist per Definition prim)
        if(n == 2)
        {
            writeln(n);
            continue;
        }

        // Naiver Test: Suche einen Teiler zwischen 2 und sqrt(n)
        for(int divisor = 2; divisor<= n/2; divisor++)
        {
            if(n % divisor == 0)
            {
                // Teiler gefunden -> n ist nicht prim
                isPrime = false;
                break;
            }
        }

        // Wenn isPrime true geblieben ist, dann ausgeben
        if(isPrime)
        {
            writeln(n);
        }
    }
}

