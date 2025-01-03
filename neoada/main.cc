#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <lexer.h>
#include <parser.h>
#include <interpreter.h>
#include <runtime.h>
#include <neoadaapi.h>

int main(int argc, char* argv[]) {
    std::string script;
    
    // Wenn ein Kommandozeilenargument übergeben wurde, dann versuchen wir,
    // dieses als Dateinamen zu interpretieren und daraus zu lesen.
    // Ansonsten lesen wir von der Standardeingabe (stdin).
    if (argc == 2) {
        std::ifstream file(argv[1], std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Fehler beim Öffnen der Datei: " << argv[1] << "\n";
            return 1;
        }
        
        // Datei vollständig in den String einlesen
        std::ostringstream buffer;
        buffer << file.rdbuf();
        script = buffer.str();
    } else {
        // Keine Argumente: von stdin lesen (kann von einer Pipe kommen)
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        script = buffer.str();
    }

    if (script.empty())
        return 0;


    // Hier kann nun "script" weiterverarbeitet werden.
    // Zum Testen geben wir es einfach aus.
    // std::cout << "Eingelesener Inhalt:\n" << script << "\n";
    
    NdaRuntime   runTime;


    runTime.state()->bindPrc("print",{{"message", "Any", Nda::InMode}}, [&](const Nda::FncValues& args) -> bool {
        std::cout << args.at("message").toString() << std::endl;
        return true;
    });

    runTime.runScript(script);

    return 0;
}

