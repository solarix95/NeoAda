# NeoAda

NeoAda is a lightweight scripting language inspired by the Ada programming language, designed for simplicity, readability, and integration with compiled languages like C++.

## **Features**

- **Ada95-inspired syntax:** Offers a familiar structure for Ada users while being intuitive for newcomers.
- **Strong typing:** Supports primitive types like `Natural`, `Number`, `String`, and open-defined containers (lists/dicts)
- **Methods:** Allows type-bound methods with a modern, flexible syntax.
- **Control structures:** Includes `if`, `for`, `while`, and Ada95-style `break when` and `continue when` constructs.
- **Extendable:** Designed for integration into compiled languages with bindings to C++.

## **Highlights**
- **Volatile datatypes:** Offers simple read/write callbacks to C++ 
- **Any datatype:** auto datatype as in C++

## **Hello, World!**
Here’s a simple NeoAda program to get started:

```neoada
declare msg : string := "Hello, world!";
print(msg);
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

### **Exceptions**
NeoAda supports simple Ada95-style exception handling. Exceptions can be raised explicitly and handled by name or with `others`.

```neoada
begin
    raise MyError;
exception
    when MyError =>
        print("handled MyError");
    when others =>
        print("handled something else");
end;
```

Handlers may re-raise either a named exception or the currently handled exception:

```neoada
begin
    raise ConstraintError;
exception
    when ConstraintError =>
        print("invalid value");
        raise;              -- re-raise current exception
    when others =>
        raise ProgramError; -- raise a specific exception
end;
```

Built-in runtime exceptions currently include:

- `ConstraintError`: arithmetic and value constraints, for example division by zero.
- `ProgramError`: invalid program state, for example failed assignment to a strongly typed variable.

Unhandled script exceptions are available to C++ callers through `NdaState::unhandledException()`.

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

## **Addon Reference**

Addons are loaded with `with Ada.Name;`. Type and method names are case-insensitive, but the examples use the preferred display style. Static methods use `Type:method(...)`; instance methods use `value.method(...)`. Instance methods can be chained.

```neoada
with Ada.String;
with Ada.DateTime;

declare start : DateTime := DateTime:fromString("2026-05-21 09:30:00", "yyyy-MM-dd HH:mm:ss");
print(start.addDays(3).addSecs(7200).toString("dd.MM.yyyy HH:mm"));
```

### **Ada.String**

Provides methods for `String` values and byte conversion helpers.

| Method | Returns | Description |
| --- | --- | --- |
| `s.length()` | `Natural` | Number of characters/bytes in the string storage. |
| `s.append(value)` | - | Appends any value converted to text. |
| `s.toUpper()` | `String` | Uppercase copy. |
| `s.toLower()` | `String` | Lowercase copy. |
| `s.upper()` | - | Converts the string in place to uppercase. |
| `s.lower()` | - | Converts the string in place to lowercase. |
| `s.contains(text)` | `Boolean` | True if `text` occurs in `s`. |
| `s.indexOf(text)` | `Natural` | First position, or `-1` if not found. |
| `s.insert(pos, text)` | - | Inserts text at position. |
| `s.trim()` | - | Trims whitespace in place. |
| `s.trimmed()` | `String` | Trimmed copy. |
| `s.chop(n)` | - | Removes `n` characters from the end in place. |
| `s.chopped(n)` | `String` | Copy without the last `n` characters. |
| `s.slice(pos, n)` | - | Keeps a slice in place. |
| `s.sliced(pos, n)` | `String` | Slice copy. |
| `String:fromBytes(data, encoding)` | `String` | Decodes `Bytes` as text. |
| `s.toBytes(encoding)` | `Bytes` | Encodes text to `Bytes`. |

Supported encodings are provided by `Ada.Text.Encoding`: `utf-8`, `utf8`, `latin1`, `iso-8859-1`, `ascii`, `utf-16`, `utf16`, `utf-16le`, `utf16le`, `utf-16be`, `utf16be`.

### **Ada.List**

Provides methods for `List` values.

| Method | Returns | Description |
| --- | --- | --- |
| `xs.length()` | `Natural` | Number of elements. |
| `xs.clear()` | - | Removes all elements. |
| `xs.append(value)` | - | Appends one element. |
| `xs.insert(pos, value)` | - | Inserts at position. |
| `xs.concat(value)` | - | Concatenates another list/value. |
| `xs.contains(value)` | `Boolean` | True if the value exists. |
| `xs.indexOf(value)` | `Natural` | First position, or `-1` if not found. |
| `xs.flip()` | - | Reverses in place. |
| `xs.flipped()` | `List` | Reversed copy. |

### **Ada.Bytes**

Provides binary byte buffers. Byte literals use the `Byte` type; indexed access uses `[]`.

| Method | Returns | Description |
| --- | --- | --- |
| `data.length()` | `Natural` | Number of bytes. |
| `data.append(byte)` | - | Appends one byte. |
| `data.append(bytes)` | - | Appends another byte buffer. |
| `data.clear()` | - | Removes all bytes. |
| `data.contains(byte)` | `Boolean` | True if the byte exists. |
| `data.indexOf(byte)` | `Natural` | First position, or `-1` if not found. |
| `data.insert(pos, byte)` | - | Inserts one byte. |
| `data.remove(pos, n)` | - | Removes `n` bytes. |
| `data.chop(n)` | - | Removes `n` bytes from the end in place. |
| `data.chopped(n)` | `Bytes` | Copy without the last `n` bytes. |
| `data.slice(pos, n)` | - | Keeps a slice in place. |
| `data.sliced(pos, n)` | `Bytes` | Slice copy. |
| `data.mid(pos, n)` | `Bytes` | Slice copy alias. |

### **Ada.Text.Encoding**

Provides explicit conversion between `String` and `Bytes`.

| Class | Method | Returns | Description |
| --- | --- | --- | --- |
| `Encoding` | `Encoding:encode(text, encoding)` | `Bytes` | Encodes text. |
| `Encoding` | `Encoding:decode(data, encoding)` | `String` | Decodes bytes. |
| `String` | `String:fromBytes(data, encoding)` | `String` | Convenience wrapper. |
| `String` | `text.toBytes(encoding)` | `Bytes` | Convenience wrapper. |

### **Ada.Math**

Provides the static `Math` class.

| Method | Returns | Description |
| --- | --- | --- |
| `Math:pi()` | `Number` | Pi constant. |
| `Math:e()` | `Number` | Euler's number. |
| `Math:tau()` | `Number` | Tau constant. |
| `Math:infinity()` | `Number` | Positive infinity. |
| `Math:nan()` | `Number` | NaN value. |
| `Math:abs(x)` | `Number` | Absolute value. |
| `Math:floor(x)` | `Number` | Floor. |
| `Math:ceil(x)` | `Number` | Ceiling. |
| `Math:round(x)` | `Number` | Round to nearest. |
| `Math:trunc(x)` | `Number` | Truncate fractional part. |
| `Math:sqrt(x)` | `Number` | Square root. |
| `Math:cbrt(x)` | `Number` | Cube root. |
| `Math:pow(x, y)` | `Number` | Power. |
| `Math:hypot(x, y)` | `Number` | Hypotenuse. |
| `Math:fmod(x, y)` | `Number` | Floating modulo. |
| `Math:remainder(x, y)` | `Number` | IEEE remainder. |
| `Math:copySign(x, y)` | `Number` | `x` with sign of `y`. |
| `Math:exp(x)` | `Number` | Exponential. |
| `Math:log(x)` | `Number` | Natural logarithm. |
| `Math:log10(x)` | `Number` | Base-10 logarithm. |
| `Math:log2(x)` | `Number` | Base-2 logarithm. |
| `Math:sin(x)`, `Math:cos(x)`, `Math:tan(x)` | `Number` | Trigonometry in radians. |
| `Math:asin(x)`, `Math:acos(x)`, `Math:atan(x)` | `Number` | Inverse trigonometry. |
| `Math:atan2(y, x)` | `Number` | Two-argument arctangent. |
| `Math:sinh(x)`, `Math:cosh(x)`, `Math:tanh(x)` | `Number` | Hyperbolic functions. |
| `Math:min(x, y)` | `Number` | Minimum. |
| `Math:max(x, y)` | `Number` | Maximum. |
| `Math:clamp(x, lo, hi)` | `Number` | Clamps `x` into range. |
| `Math:sign(x)` | `Number` | `-1`, `0`, or `1`. |
| `Math:radians(degrees)` | `Number` | Degrees to radians. |
| `Math:degrees(radians)` | `Number` | Radians to degrees. |
| `Math:isNan(x)` | `Boolean` | True for NaN. |
| `Math:isFinite(x)` | `Boolean` | True for finite numbers. |
| `Math:isInf(x)` | `Boolean` | True for infinity. |

### **Ada.Io.File**

Provides binary `File` and text `TextFile`. `with Ada.Io;` is accepted as a shorthand. Both types are dictionary-backed objects with members such as `path`, `mode`, `encoding`, and `closed`.

| Class | Method | Returns | Description |
| --- | --- | --- | --- |
| `File` | `File:open(path)` | `File` | Opens binary read/write. |
| `File` | `File:openRead(path)` | `File` | Opens binary read-only. |
| `File` | `File:create(path)` | `File` | Creates/truncates binary file. |
| `File` | `File:append(path)` | `File` | Opens binary append. |
| `File` | `File:exists(path)` | `Boolean` | File existence check. |
| `File` | `f.isOpen()` | `Boolean` | True if a handle is open. |
| `File` | `f.close()` | - | Closes the handle. |
| `File` | `f.flush()` | - | Flushes buffered data. |
| `File` | `f.eof()` | `Boolean` | End-of-file state. |
| `File` | `f.write(data)` | - | Writes `Bytes`. |
| `File` | `f.readAll()` | `Bytes` | Reads all bytes. |
| `TextFile` | `TextFile:open(path)` | `TextFile` | Opens text read/write. |
| `TextFile` | `TextFile:openRead(path)` | `TextFile` | Opens text read-only. |
| `TextFile` | `TextFile:create(path)` | `TextFile` | Creates/truncates text file. |
| `TextFile` | `TextFile:append(path)` | `TextFile` | Opens text append. |
| `TextFile` | `TextFile:exists(path)` | `Boolean` | File existence check. |
| `TextFile` | `f.isOpen()` | `Boolean` | True if a handle is open. |
| `TextFile` | `f.close()` | - | Closes the handle. |
| `TextFile` | `f.flush()` | - | Flushes buffered data. |
| `TextFile` | `f.eof()` | `Boolean` | End-of-file state. |
| `TextFile` | `f.write(text)` | - | Writes string data. |
| `TextFile` | `f.writeLine(text)` | - | Writes a line plus newline. |
| `TextFile` | `f.readAll()` | `String` | Reads all text. |
| `TextFile` | `f.readLine()` | `String` | Reads one line. |

### **Ada.DateTime**

Provides dictionary-backed `Date`, `Time`, and `DateTime` objects. Supported format tokens are `yyyy`, `yy`, `MM`, `dd`, `HH`, `mm`, and `ss`. Literal separators are copied/matched exactly. `with Ada.Date`, `with Ada.Date.Time`, and `with Ada.DateTime` are accepted.

| Class | Method | Returns | Description |
| --- | --- | --- | --- |
| `Date` | `Date:now()` | `Date` | Current local date. |
| `Date` | `Date:today()` | `Date` | Alias for current local date. |
| `Date` | `Date:fromString(text, format)` | `Date` | Parses a date. |
| `Date` | `d.toString(format)` | `String` | Formats a date. |
| `Date` | `d.addDays(days)` | `Date` | Returns date plus days. |
| `Time` | `Time:now()` | `Time` | Current local time. |
| `Time` | `Time:fromString(text, format)` | `Time` | Parses a time. |
| `Time` | `t.toString(format)` | `String` | Formats a time. |
| `Time` | `t.addSecs(secs)` | `Time` | Returns time plus seconds. |
| `DateTime` | `DateTime:now()` | `DateTime` | Current local date and time. |
| `DateTime` | `DateTime:fromString(text, format)` | `DateTime` | Parses date and time. |
| `DateTime` | `dt.toString(format)` | `String` | Formats date and time. |
| `DateTime` | `dt.addDays(days)` | `DateTime` | Returns date/time plus days. |
| `DateTime` | `dt.addSecs(secs)` | `DateTime` | Returns date/time plus seconds. |

### **Core Dictionary Type**

`Dict` is a core container type, not a separate addon. Use dictionary literals and `{}` access for arbitrary key/value members.

```neoada
declare person : Dict := {"name": "Ada", "age": 42};
print(person{"name"});
```

## **Planned Features**
- Advanced data structures (e.g., dictionaries, dynamic arrays).
- Native bindings for Python and Java.
- Enhanced error diagnostics.

## **Contributing**
Contributions are welcome! Please see the `CONTRIBUTING.md` for guidelines on how to help improve NeoAda.

## **License**
NeoAda is open source and available under the MIT License.

