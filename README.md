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

---

## Editor & IDE Support

For editor support, syntax highlighting, language features, and code intelligence, use the official Visual Studio Code extension:

* **[VS Code Stride Extension](https://github.com/StrideLang/vscode-stride-lang)** (`vscode-stride-lang`)

---

## Building from Source

### Prerequisites

* **CMake** 3.15 or newer
* **C++17** compatible compiler (MSVC 2019/2022, GCC 9+, or Clang 10+)
* **Qt 6** (6.5+ recommended)
* **Flex** & **Bison**

### Clone Repository

Clone recursively to fetch all submodules:

```bash
git clone --recurse-submodules git@github.com:StrideLang/Stride.git
cd Stride
```

If already cloned, initialize and update submodules:

```bash
git submodule update --init --recursive
```

### Platform-Specific Setup

#### macOS

```bash
brew install bison flex qt@6 cmake
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake flex bison qt6-base-dev libgl1-mesa-dev
```

#### Windows

1. Install Visual Studio 2019 or 2022 with C++ desktop development.
2. Install [Qt 6](https://www.qt.io/download).
3. Install Flex and Bison:
   * [Flex for Windows](http://gnuwin32.sourceforge.net/packages/flex.htm)
   * [Bison for Windows](http://gnuwin32.sourceforge.net/packages/bison.htm)
4. When configuring CMake, specify paths to Flex and Bison if not in `PATH`:
   ```cmd
   cmake -B build -DBISON_EXECUTABLE="C:\Program Files (x86)\GnuWin32\bin\bison.exe" -DFLEX_EXECUTABLE="C:\Program Files (x86)\GnuWin32\bin\flex.exe"
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
