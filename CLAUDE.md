# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working in this repository.

## Build and Run

```sh
# Build (Debug)
xcodebuild -project HelloWorld.xcodeproj -scheme HelloWorld -configuration Debug build

# Build (Release)
xcodebuild -project HelloWorld.xcodeproj -scheme HelloWorld -configuration Release build

# Run after building (requires Xcode → Settings → Locations → Derived Data set to "Relative")
./DerivedData/HelloWorld/Build/Products/Debug/HelloWorld

# If using the default "Unique" Derived Data location:
# ~/Library/Developer/Xcode/DerivedData/HelloWorld-<hash>/Build/Products/Debug/HelloWorld
```

There are no tests in this project.

## Architecture

This is a macOS command-line tool (single Xcode target: `HelloWorld`) with two purposes:

1. **Cross-language interop demo** — `Source/main.swift` calls a `hello_*` function implemented in each of the four supported languages (C, C++, Objective-C, Swift), then calls a `run_demo_*` function from each language that exercises a richer set of constructs.

2. **Coding standards reference** — each `Demo_*` file is written to illustrate the project's coding style rules and to surface the style decisions that tend to generate the most debate. See `~/.claude/CLAUDE.md` for the global style guide.

### Cross-language bridging

Swift calls into C, C++, and Objective-C via `Source/HelloWorld-Bridging-Header.h`, which imports all three language headers. The C++ headers wrap their declarations in `extern "C"` guards so symbols are accessible without C++ name mangling.

### File reference

| File | Role |
|---|---|
| `main.swift` | Entry point — calls all `hello_*` and `run_demo_*` functions |
| `HelloWorld.swift` | Swift hello function |
| `HelloWorld.c` / `.h` | C hello function |
| `HelloWorld.cpp` / `.hpp` | C++ hello function (`extern "C"` guard in header) |
| `HelloWorld.m` / `HelloWorld_objc.h` | Objective-C hello function |
| `HelloWorld-Bridging-Header.h` | Exposes C, C++, and Obj-C symbols to Swift |
| `Demo_C.h` / `Demo_C.c` | C demo: `Student` struct, error codes, function-pointer visitor/predicate pattern |
| `Demo_Cpp.hpp` / `Demo_Cpp.cpp` | C++ demo: `Grade_Book` class, lambdas, `std::optional`, `std::format` |
| `Demo_ObjC.h` / `Demo_ObjC.m` | Objective-C demo: `HWGradeBook`, block typedefs, `__weak`/`__strong` pattern |
| `Demo_Swift.swift` | Swift demo: enums, `GradeBook` class, closures, `Result`, `@escaping` |

### Note on C++ standard

The Xcode project sets `CLANG_CXX_LANGUAGE_STANDARD = gnu++23`. The demo code uses `std::format` (C++20) and `std::optional` (C++17); both are available under `gnu++23` on recent Apple SDKs.
