# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working in this repository.

## Build and Run

```sh
# Build (Debug)
xcodebuild -project HelloWorld.xcodeproj -scheme Tool -configuration Debug build

# Build (Release)
xcodebuild -project HelloWorld.xcodeproj -scheme Tool -configuration Release build

# Run after building (requires Xcode → Settings → Locations → Derived Data set to "Relative")
./DerivedData/HelloWorld/Build/Products/Debug/HelloWorld

# If using the default "Unique" Derived Data location:
# ~/Library/Developer/Xcode/DerivedData/HelloWorld-<hash>/Build/Products/Debug/HelloWorld
```

The scheme is `Tool`, not `HelloWorld` — see Architecture below — but the built binary is still named `HelloWorld`, so the run paths are unchanged.

`Tool` is the only scheme, and it is **shared** — tracked at `HelloWorld.xcodeproj/xcshareddata/xcschemes/Tool.xcscheme`, so it exists in a fresh clone and in CI instead of being synthesized per-user.

There are no tests in this project.

## Architecture

This is a macOS command-line tool with two purposes:

1. **Cross-language interop demo** — `Source/main.swift` calls a `hello_*` function implemented in each of the two supported languages (C++, Swift), then calls a `run_demo_*` function from each language that exercises a richer set of constructs.

2. **Coding standards reference** — each language's `HelloWorld.*` file is written to illustrate the project's coding style rules and to surface the style decisions that tend to generate the most debate. See `~/.claude/CLAUDE.md` for the global style guide.

One target, named generically after what it builds rather than after its product:

| Target | Product | Notes |
|---|---|---|
| `Tool` | `HelloWorld` | The command-line tool. Builds from `Source/` |

The name matches the `Config/` filenames (`Tool-Common.xcconfig` and friends) rather than the product, which is the same convention the Utilities project uses for its `Framework` and `Tests` targets. `PRODUCT_NAME` is pinned to `HelloWorld` in `Tool-Common.xcconfig`, so the binary is unaffected by the target's name.

### Languages

The tool is configured to host **C, C++, Objective-C, and Swift**, not just the two it currently uses. `Source/` contains only C++ and Swift — the C and Objective-C sources were deliberately removed — but the build settings are broader than its present contents on purpose: `GCC_C_LANGUAGE_STANDARD = gnu23`, `CLANG_CXX_LANGUAGE_STANDARD = gnu++23`, ARC and weak references enabled, strict `objc_msgSend` prototypes, and the full Objective-C and C++ warning and static-analyzer allowlists.

Do not read the current file list as the project's language scope, and do not prune C, C++, or Objective-C settings as dead weight. Apple frequently reuses a build setting that is ostensibly for one language to control another, and frequently does not document that it does — so a setting that looks inert for an absent language may be governing the languages that *are* compiled here.

Platform scope is a separate axis and *is* narrow: `SUPPORTED_PLATFORMS = macosx` alone, because a macOS command-line tool cannot meaningfully target anything else. That narrowing says nothing about the language settings above.

### Cross-language bridging

Swift calls into C++ via `Source/HelloWorld-Bridging-Header.h`, which imports the C++ header. That header wraps its declarations in an `extern "C"` guard so symbols are accessible without C++ name mangling. The guard is load-bearing: Swift's ClangImporter parses the bridging header in C mode, where `__cplusplus` is undefined, so it sees plain C declarations while the C++ translation unit sees `extern "C"` linkage.

The bridging header deliberately carries **no include guard**. The ClangImporter compiles it as a translation unit's main file rather than including it, so a `#pragma once` there draws `-Wpragma-once-outside-header` — an error under `GCC_TREAT_WARNINGS_AS_ERRORS`. Nothing includes the bridging header, so it needs no guard; `HelloWorld.hpp`, which really is included, keeps its own.

### File reference

Each language has a single source/header pair containing both its `hello_*` function and a `run_demo_*` function. Public headers expose only those two entry points; all supporting types and helpers are kept private to the implementation file.

| File | Role |
|---|---|
| `main.swift` | Entry point — calls all `hello_*` and `run_demo_*` functions |
| `HelloWorld.hpp` / `HelloWorld.cpp` | C++: `hello_cpp`, `run_demo_cpp`; private `Grade_Book` class, lambdas, `std::optional`, `std::format` |
| `HelloWorld.swift` | Swift: `helloSwift`, `runDemoSwift`; enums, `GradeBook` class, closures, `Result`, `@autoclosure` |
| `HelloWorld-Bridging-Header.h` | Exposes the C++ entry points to Swift |

### Adding files

`Source/` and `Config/` are `PBXFileSystemSynchronizedRootGroup`s (`objectVersion = 77`). Files are picked up by folder membership — **drop a file into the directory and it joins the target with no `project.pbxproj` edit**. This is also why XCODE-1 (navigator mirrors the filesystem) holds by construction here; there is no way to create a virtual group that diverges from disk.

### Build settings (`Config/`)

All Xcode build settings live in `.xcconfig` files under `Config/` rather than inline in `project.pbxproj`. The four `XCBuildConfiguration`s reference these files through `baseConfigurationReferenceAnchor` + `baseConfigurationReferenceRelativePath`, and their inline `buildSettings` dicts are empty — every setting flows from a tracked text file.

| File | Scope |
|---|---|
| `Project-Common.xcconfig` | Everything shared project-wide: language standards, the full warning allowlist, static analyzer, Swift language mode and concurrency, signing, versioning |
| `Project-Debug.xcconfig` / `Project-Release.xcconfig` | `#include` Project-Common, then per-configuration overrides (optimization, testability, dSYM, stripping) |
| `Tool-Common.xcconfig` | Tool target: SDK and platform, deployment target, packaging, bundle ID, product name, bridging header |
| `Tool-Debug.xcconfig` / `Tool-Release.xcconfig` | `#include` Tool-Common, plus the target-level `HELLOWORLD_DEBUG` / `HELLOWORLD_RELEASE` flags |

When changing a build setting, edit the appropriate `.xcconfig` file rather than the project's "Build Settings" tab in Xcode — anything set in the UI gets written back as an inline override in `project.pbxproj` and silently shadows the xcconfig value. If a `buildSettings` dict in `project.pbxproj` is ever non-empty, Xcode's UI put it there.

#### How these files are organised

**The commented sections reproduce the organization of Xcode's Build Settings UI**, so a setting seen in the UI can be found in the xcconfig and vice versa. Preserve this; do not regroup by any other logic.

- Section headers are Xcode's category names verbatim, written as `//`, a tab, then the name.
- General categories come first alphabetically (`Architectures`, `Build Options`, `Deployment`, `Info.plist Values`, `Linking - General`, `Localization`, `Packaging`, `Search Paths`, `Signing`, `Versioning`), then the tool-prefixed groups alphabetically (`Apple Clang - *`, `Metal Compiler - *`, `Static Analysis - *`, `Swift Compiler - *`), with `Miscellaneous` last.
- Settings are alphabetised within a section.
- Single space around `=`. These files deliberately do **not** use ALL-6 column alignment.

Adding a setting therefore means finding its Xcode category and placing it there alphabetically — never appending to the end of the file.

Settings worth knowing about:

- `SWIFT_TREAT_WARNINGS_AS_ERRORS = YES` and `GCC_TREAT_WARNINGS_AS_ERRORS = YES` — any new warning fails the build.
- `RUN_CLANG_STATIC_ANALYZER = YES` — the analyzer runs on every build, not just on Analyze.
- `SWIFT_VERSION = 6.0` with `SWIFT_STRICT_CONCURRENCY = complete` and `SWIFT_DEFAULT_ACTOR_ISOLATION = nonisolated`.
- `MACOSX_DEPLOYMENT_TARGET = 26.0` in `Tool-Common.xcconfig`, matching Utilities. Nothing in the current code requires that floor, so it could be lowered if the tool ever needs to run on an older system.
- `SWIFT_DISABLE_SAFETY_CHECKS = NO`, stated explicitly even though `NO` is Xcode's default, so bounds checks, overflow traps, and preconditions survive in Release.
- No DocC. This project is documented by `README.md` and `CLAUDE.md` and publishes no API surface, so `RUN_DOCUMENTATION_COMPILER` and the `DOCC_*` settings are deliberately absent. Do not add them for consistency with other projects. Note that `CLANG_WARN_DOCUMENTATION_COMMENTS` is *not* one of them — it is a Clang warning about malformed doc comments and is enabled.

### Note on C++ standard

The Xcode project sets `CLANG_CXX_LANGUAGE_STANDARD = gnu++23`. The demo code uses `std::format` (C++20) and `std::optional` (C++17); both are available under `gnu++23` on recent Apple SDKs.
