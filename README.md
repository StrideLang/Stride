# Stride

[![Build Status](https://travis-ci.org/StrideLanguage/Stride.svg?branch=master)](https://travis-ci.org/StrideLanguage/Stride)

This is the central development repository for the **Stride** programming language ecosystem. It orchestrates the core components, libraries, runtimes, and command-line tools into an integrated development workspace.

---

## Ecosystem Overview

Stride is built modularly across specialized repositories integrated here as submodules:

| Component | Repository | Description |
| :--- | :--- | :--- |
| **`stridelang`** | [StrideLang/stridelang](https://github.com/StrideLang/stridelang) | The main compiler driver and command-line frontend (`stridec`). |
| **`strideparser`** | [StrideLang/strideparser](https://github.com/StrideLang/strideparser) | Lexer, parser, and Abstract Syntax Tree (AST) generation. |
| **`strideutils`** | [StrideLang/strideutils](https://github.com/StrideLang/strideutils) | Shared utilities, data structures, and common helper libraries. |
| **`codegen`** | [StrideLang/codegen](https://github.com/StrideLang/codegen) | Target code generation engine. |
| **`stridejit`** | [StrideLang/stridejit](https://github.com/StrideLang/stridejit) | JIT compilation engine and runtime execution environment. |
| **`strideroot`** | [StrideLang/strideroot](https://github.com/StrideLang/strideroot) | Core platform definitions, runtime libraries, and target frameworks. |
| **`stridemanager`** | [StrideLang/stridemanager](https://github.com/StrideLang/stridemanager) | Project and tool configuration manager CLI (`stridemngr`). |
| **`editor`** | [StrideLang/editor](https://github.com/StrideLang/editor) | Legacy standalone desktop IDE (`StrideIDE` - deprecated). |

---

## Editor & IDE Support

For active editor support, syntax highlighting, language features, and code intelligence, use the official Visual Studio Code extension:

* **[VS Code Stride Extension](https://github.com/StrideLang/vscode-stride-lang)** (`vscode-stride-lang`)

---

## Prerequisites & Dependencies

Building Stride requires standard build tools alongside specific third-party libraries mapped to each submodule:

### General Requirements

* **CMake** (>= 3.15) — Required for all submodules.
* **C++17 Compiler** — MSVC 2019/2022, GCC 9+, or Clang 10+ across all modules.

### Submodule Dependencies

| Dependency | Required By | Purpose |
| :--- | :--- | :--- |
| **LLVM** (14.x recommended) | **`stridejit`** | Provides JIT compilation (ORC JIT, ExecutionEngine, JITLink) and multi-target code generation (x86, AArch64, ARM, WebAssembly). |
| **Flex & Bison** | **`strideparser`** | Generates the lexical scanner (`lang_stride.l`) and LALR parser (`lang_stride.y`) for Stride source code. |
| **Platform Toolchains** *(Optional)* | **`strideroot`** | Target-specific cross-compilers (e.g. ARM GCC for STM32, XMOS XTC, Arduino/Wiring, RtAudio) when building for embedded platforms. |

---

## Building from Source

### Clone Repository

Clone recursively to fetch all submodules:

```bash
git clone --recurse-submodules https://github.com/StrideLang/Stride.git
cd Stride
```

If already cloned, initialize and update submodules:

```bash
git submodule update --init --recursive
```

### Platform-Specific Setup

#### macOS

Install build dependencies using Homebrew:

```bash
brew install cmake bison flex llvm@14
```

Set paths if using Homebrew-installed Bison and LLVM:
```bash
export PATH="$(brew --prefix bison)/bin:$(brew --prefix llvm@14)/bin:$PATH"
export LLVM_DIR="$(brew --prefix llvm@14)/lib/cmake/llvm"
```

#### Linux (Ubuntu/Debian)

Install build dependencies via `apt`:

```bash
sudo apt update
sudo apt install -y build-essential cmake flex bison \
    llvm-14-dev libllvm14
```

#### Windows

1. Install **Visual Studio 2019 or 2022** with the **Desktop development with C++** workload.
2. Install **Flex and Bison**:
   * Download and extract [win_flex_bison](https://sourceforge.net/projects/winflexbison/).
3. Install **LLVM 14**:
   * Build/install LLVM 14 from source (e.g. `llvm-project` branch `llvmorg-14.0.6`) or install pre-built binaries.
4. When configuring CMake, supply paths to LLVM, Flex, and Bison:
   ```cmd
   cmake -B build -DCMAKE_BUILD_TYPE=Release ^
       -DLLVM_DIR="C:/path/to/llvm-install/lib/cmake/llvm" ^
       -DBISON_EXECUTABLE="C:/path/to/win_flex_bison/win_bison.exe" ^
       -DFLEX_EXECUTABLE="C:/path/to/win_flex_bison/win_flex.exe"
   ```

---

### Build Instructions

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```

---

## License

Stride is licensed under the terms of the 3-clause BSD License. See `License.txt` for details.
