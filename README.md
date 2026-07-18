# HelloWorld

This project serves two purposes:

1. **Cross-language interop demo** — a macOS command-line tool built with Xcode that demonstrates calling functions written in C++ and Swift from a shared Swift entry point.

2. **Coding standards reference** — each language's `HelloWorld.*` file exercises the constructs most relevant to the project's coding style rules, and deliberately includes the patterns that tend to generate the most debate about the "right" way to write them.

## Languages and constructs covered

| Language | Key constructs demonstrated |
|---|---|
| **C++** | Class with Rule of 6, `std::optional`, `std::function` callbacks, stored lambda, capture-by-value vs reference, mutable lambda, immediately invoked lambda, `std::format`, template function |
| **Swift** | `enum` with raw values, associated values, and `CaseIterable`; `struct` vs `class`; `@escaping` closure; `[weak self]` capture; trailing closure syntax; `Result`; exhaustive `switch`; `@autoclosure` |

## Building and running

```sh
# Build
xcodebuild -project HelloWorld.xcodeproj -scheme HelloWorld -configuration Debug build

# Run (with Derived Data set to "Relative" in Xcode → Settings → Locations)
./DerivedData/HelloWorld/Build/Products/Debug/HelloWorld
```

See `CLAUDE.md` for full build instructions and architecture details.
