Create a new SIERA core module named `$ARGUMENTS`.

Follow these steps exactly:

1. Create `src/core/$ARGUMENTS/$ARGUMENTS.h` with:
   - `#pragma once`
   - Any necessary includes (at minimum `<stdint.h>` and `<stdbool.h>` if needed)
   - Public struct typedefs following the `<name>_t` convention
   - Public function declarations following `<module>_<action>()` naming

2. Create `src/core/$ARGUMENTS/$ARGUMENTS.c` with:
   - `#include "$ARGUMENTS.h"` as the first include
   - Stub implementations of all declared functions

Do NOT modify any CMakeLists.txt — sources are auto-globbed.

Remind the user to add a `target_include_directories` entry in `src/core/CMakeLists.txt` if the module introduces a new public include path.
