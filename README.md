# NeoAda

NeoAda is a lightweight scripting language inspired by Ada95, designed for simplicity, readability, and integration with compiled languages like C++.

## **Features**

- **Ada95-inspired syntax:** Offers a familiar structure for Ada users while being intuitive for newcomers.
- **Strong typing:** Supports primitive types like `Natural`, `Number`, `String`, and open-defined containers (lists/dicts)
- **Methods:** Allows type-bound methods with a modern, flexible syntax.
- **Control structures:** Includes `if`, `for`, `while`, and Ada95-style `break when` and `continue when` constructs.
- **Extendable:** Designed for integration into compiled languages with bindings to C++.

## **Hello, World!**
Here’s a simple NeoAda program to get started:

```neoada
function string:length() return Natural is
begin
    return 11;
end;

declare msg : string := "Hello, world!";
print(msg.length());
```

## **Core Syntax Overview**

### **Variable Declaration**
```neoada
declare x : Natural := 42;
```

### **Control Structures**
#### If Statements
```neoada
if x > 10 then
    print("x is greater than 10");
elsif x = 10 then
    print("x is equal to 10");
else
    print("x is less than 10");
end if;
```

#### For Loops
```neoada
for i in 1..10 loop
    print(i);
end loop;
```

#### While Loops
```neoada
while x > 0 loop
    x := x - 1;
end loop;
```

### **Method Declaration and Calls**
#### Instance Method
```neoada
function string:length() return Natural is
begin
    return 11;
end;

-- Call
declare msg : string := "Hello!";
print(msg.length());
```

#### Static Method
```neoada
function string:format(value : Number, fmt : String) return String is
begin
    -- Formatting logic
end;

-- Call
print(string:format(42.2, "%.1f"));
```

### **Comments**
NeoAda supports Ada95-style line comments:
```neoada
-- This is a comment
```

## **Use Cases**

NeoAda is ideal for:
- **Embedding in compiled applications:** Simplify configuration and scripting for binaries.
- **Educational purposes:** A beginner-friendly language for learning programming concepts.
- **Scripting algorithms and data structures:** Offload dynamic logic while leveraging compiled performance.

## **Integration with C++**
NeoAda offers seamless integration with C++ via a clean API for parsing and executing scripts. Static methods and functions are registered in a centralized function table for runtime execution.

### Example
```cpp
#include "nada_lexer.h"
#include "nada_parser.h"
#include "nada_runtime.h"

int main() {
    NeoAda::Runtime runtime;
    runtime.execute("print(\"Hello from NeoAda!\");");
    return 0;
}
```

## **Planned Features**
- Advanced data structures (e.g., dictionaries, dynamic arrays).
- Native bindings for Python and Java.
- Enhanced error diagnostics.

## **Contributing**
Contributions are welcome! Please see the `CONTRIBUTING.md` for guidelines on how to help improve NeoAda.

## **License**
NeoAda is open source and available under the MIT License.

