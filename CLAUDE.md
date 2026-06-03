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

2. **Coding standards reference** — each language's `HelloWorld.*` file is written to illustrate the project's coding style rules and to surface the style decisions that tend to generate the most debate. See `~/.claude/CLAUDE.md` for the global style guide.

### Cross-language bridging

Swift calls into C, C++, and Objective-C via `Source/HelloWorld-Bridging-Header.h`, which imports all three language headers. The C++ header wraps its declarations in an `extern "C"` guard so symbols are accessible without C++ name mangling.

### File reference

Each language has a single source/header pair containing both its `hello_*` function and a `run_demo_*` function. Public headers expose only those two entry points; all supporting types and helpers are kept private to the implementation file.

| File | Role |
|---|---|
| `main.swift` | Entry point — calls all `hello_*` and `run_demo_*` functions |
| `HelloWorld.h` / `HelloWorld.c` | C: `hello_c`, `run_demo_c`; private `Student` struct, error codes, function-pointer visitor/predicate pattern |
| `HelloWorld.hpp` / `HelloWorld.cpp` | C++: `hello_cpp`, `run_demo_cpp`; private `Grade_Book` class, lambdas, `std::optional`, `std::format` |
| `HelloWorld_objc.h` / `HelloWorld.m` | Objective-C: `hello_objc`, `run_demo_objc`; private `HWGradeBook`, block typedefs, `__weak`/`__strong` pattern |
| `HelloWorld.swift` | Swift: `helloSwift`, `runDemoSwift`; enums, `GradeBook` class, closures, `Result`, `@escaping` |
| `HelloWorld-Bridging-Header.h` | Exposes C, C++, and Obj-C entry points to Swift |

### Note on C++ standard

The Xcode project sets `CLANG_CXX_LANGUAGE_STANDARD = gnu++23`. The demo code uses `std::format` (C++20) and `std::optional` (C++17); both are available under `gnu++23` on recent Apple SDKs.
