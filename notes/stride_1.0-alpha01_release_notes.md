# Stride 1.0-alpha01 Release Notes

Below is an overview of the key language, runtime, and infrastructure improvements introduced in this release.

---

## 1. Cleaner, Less Verbose Syntax

* **Stream-Signature Callables:** Declare `module`, `reaction`, and `loop` components directly using stream flow syntax (e.g., `module In >> Gain(gain: 1.0) >> Out`), removing boilerplate `ports:` definitions.
* **Unified `code:` Blocks:** Declarations and streams can now be declared together inside a unified `code:` block, avoiding separation between declarations and stream pipelines.
* **Generator & Range Expressions:** Introduced compile-time range generator syntax (such as `[start..end]` and `[0..2..30]`) across streams, lists, and properties to simplify polyphonic routing, multichannel setups, and filter banks.
* **Modernized Nomenclature & Literals:** Standardized on `entity` and `entities:` across AST and declaration models, replacing legacy `block` / `blocks:` syntax. Added full support for binary (`0b...`), hexadecimal (`0x...`), unary minus parsing, and trailing commas.

---

## 2. Hierarchical State Machines & Discrete Control

* **UML State Machine lowered to LLVM IR:** Full compiler lowering for complex state machine models, including nested states, concurrent composite states (`isParallel`), final states (`isFinal`), and deep history restoration (`resumeLastState`).
* **Conditioned & Parameterized Transitions:** Direct evaluation of transition guards (`updateGuard`) and parameterized transitions with state hierarchy ancestry traversal.

---

## 3. Streaming Dataflow, Buffering & Multi-Rate Processing

* **First-Class `buffer` Entities:** Native sliding and circular buffers with automatic mirror allocation for boundary-safe, zero-copy frame reads (`Buf.frame`) into downstream reactions and loops.
* **Transparent Multi-Rate Streams:** Topological rate staging resolves multi-rate stream chains during compilation. Downsampling decimation branches and upsampling execution loops are generated automatically without manual phase-timer math.
* **Dynamic Sizing & Automatic Broadcasting:** Output ports dynamically adapt to consumer dimensions, and array/bundle connections automatically lower to scalar input ports via broadcast semantics.
* **Lifecycle Stream Hooks:** Native `onLoad` (one-shot per program execution) and `onInit` (per domain instance) stream hooks.

---

## 4. Hardware Specialization & Quality Policies

* **Hardware-Specific Algorithmic Variants:** Automated resolution for platform variants (e.g., ROM table-driven math, polynomial approximation, or hardware intrinsics).
* **Annotated Symbol Binding:** Automatic compile-time resolution of declarations tagged with `@Device` or `@Policy` metadata.

---

## 5. Typing, Schemas & Zero-Dependency Parsers

* **Centralized Schema Enforcement:** Declarative validation rules in `Schema.stride` enforcing collection constraints (`contains`, `uniqueBy`), numeric boundaries (`multipleOf`, `allowedValues`), and visibility/mutability (`public`, `writeable`).
* **Lossy Conversion Auditing:** Compile-time warnings for narrowing conversions and automatic typecast resolution across stream elements.
* **Standalone Header-Only Parsers:** `StrideReader` and `StrideDocument` (`stride_data.h`) provide fast data parsing and validation without requiring the full compiler runtime.

---

## 6. Compiler Engine, State Safety & Debugging

* **Source-Level DWARF Debugging:** Native line-by-line debugging emitted with DWARF metadata for step-in and step-over inspection in GDB and LLDB.
* **Zero Global Pollution:** Re-entrant, stack-allocated `__state` structs eliminate global state pollution for nested callables and multi-rate accumulators.
* **Modular Runtime:** Dedicated `StrideRuntime` static library with zero-copy `StrideStringView`, format strings, and variadic concatenation.
* **Caching:** Disk-based AST and compiled object caching.

---

## 7. Toolchain, Build Systems & Verification

* **Unified `stridec` CLI:** Ahead-of-time compiler driver supporting multi-file compilation, optimization levels (`-O0` to `-O3`), and automated C header generation (`--emit-header`).
* **Native Build System Rules:** First-class support for **CMake** (`stride_cc_library`) and **Bazel** (`rules_stride`), alongside export targets for bare-metal ARM and Arduino.
* **Differential DSP Testing:** `stridetest` test runner featuring statistical signal verification (`ExpectSnr`, `ExpectMaxError`) alongside latency, jitter, and steady-state zero-heap allocation tracking.