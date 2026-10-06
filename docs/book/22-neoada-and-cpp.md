# 22. NeoAda and C++

NeoAda is designed for embedding scripts inside compiled C++ applications. The host keeps privileged and performance-sensitive behavior while scripts express selected logic.

## Minimal embedding

The project README currently shows:

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

Verify header/API names against the version of `libneoada` you build.

## Why embed?

Typical uses include configuration, application rules, educational sandboxes, and algorithms that change more often than the host binary.

## Failure ownership

The host owns the process and therefore owns the policy for script failures: reject startup, disable one feature, fall back, or report an error.

## Version the contract

Once scripts depend on a host API, that API is a compatibility contract. Document and test it.

## Exercises

1. List three host capabilities scripts may use and three they should not.
2. Define a failure policy for an invalid startup script.
