Create a CppUTest test file for the SIERA core module `$ARGUMENTS`.

## Step 1 — Read the module API

Read `src/core/$ARGUMENTS/$ARGUMENTS.h` to understand all public structs, interfaces, and functions before writing anything.

Also read any interface headers the module depends on (e.g. `i_<dep>.h`) to understand what test doubles are needed.

## Step 2 — Create missing test doubles

For every `i_<name>_t` interface the module depends on, check whether `tests/doubles/core/<name>/double_<name>.h` and `.c` already exist. If not, create them following this pattern:

**`tests/doubles/core/<name>/double_<name>.h`**
```c
#pragma once
#include "i_<name>.h"

typedef struct {
  i_<name>_t interface;  // must be first member
  // controllable state fields (e.g. return values, call counts)
} double_<name>_t;

void double_<name>_init(double_<name>_t* self);
// setter/helper functions to control behaviour from tests
```

**`tests/doubles/core/<name>/double_<name>.c`**
```c
#include "double_<name>.h"

// Static function implementing each function pointer, casting instance to double_<name>_t*
// Init assigns all function pointers and zeroes state
```

Rules for test doubles:
- The `i_<name>_t interface` field must be the **first** member so casting `i_<name>_t*` → `double_<name>_t*` is valid
- Implement every function pointer in the interface
- Expose helpers like `double_<name>_set_<field>()` so tests can control return values
- Use `mock().actualCall(...)` only when the double is a spy/mock; for simple stubs just store/return state

## Step 3 — Create the test file

Create `tests/core/$ARGUMENTS/test_$ARGUMENTS.cpp` with:

- `#include "CppUTest/TestHarness.h"` and `#include "CppUTestExt/MockSupport.h"`
- `extern "C"` block importing the module header and all test double headers
- If the module invokes callbacks: a `static void mock_callback(void* context)` calling `mock().actualCall("callback").withPointerParameter("context", context)`
- A `TEST_GROUP($ARGUMENTSTests)` with:
  - Member variables for the module under test and all doubles
  - `setup()` initialising every double and the module
  - `teardown()` calling `mock().clear()`

For each public function write tests in all four categories:

**Happy path** — valid inputs, expected outputs/state changes.

**Error conditions** — null pointers, uninitialized state, out-of-range arguments; verify defined/safe behaviour.

**Boundary values** — min/max numeric ranges, empty/full containers, zero intervals, single-element collections.

**Mock verification** — `mock().expectOneCall(...).withPointerParameter(...)` before the action, `mock().checkExpectations()` after; verify context is forwarded and callbacks fire the correct number of times.

## Rules

- Do NOT modify any CMakeLists.txt or production source files.
- Do NOT use `malloc` in test doubles — static allocation only.
- Follow 2-space indent, `#pragma once`, and all conventions from CLAUDE.md.
