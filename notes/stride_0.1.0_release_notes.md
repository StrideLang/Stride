# Stride 0.1.0 Release Notes

Below is an overview of the key language, runtime, standard library, and toolchain improvements introduced in this release.

---

## 1. Cleaner, Less Verbose Syntax & Conditional Compilation

* **Stream-Signature Callables:** Declare `module`, `reaction`, and `loop` components directly using stream flow syntax (e.g., `module In >> Gain(gain: 1.0) >> Out`), removing boilerplate `ports:` definitions.
* **Unified `code:` Blocks:** Declarations and streams can now be declared together inside a unified `code:` block, avoiding separation between declarations and stream pipelines.
* **Generator & Range Expressions:** Introduced compile-time range generator syntax (such as `[start..end]` and `[0..2..30]`) across streams, lists, and properties to simplify polyphonic routing, multichannel setups, and filter banks.
* **Compile-Time Feature Toggles (`@with` & `@force`):** First-class `feature` declarations and exclusive variant groups with conditional `@with` property blocks, conditional `@with` package imports, and `@force` property overrides resolved at compile time via `FeatureResolver`.
* **Modernized Nomenclature & Typed Literals:** Standardized on `entity` / `entities:` and `array` across AST and declaration models, replacing legacy `block` / `blocks:` and `bundle` syntax. Added full support for typed numeric literals with destination-type widening, binary (`0b...`) and hexadecimal (`0x...`) literals, mid-stream member access, property-less entity declarations, unary minus parsing, and trailing commas.

---

## 2. Hierarchical State Machines & Discrete Control

* **UML State Machine Lowered to LLVM IR:** Full compiler lowering for complex state machine models, including nested states, concurrent composite states (`isParallel`), final states (`isFinal`), and deep history restoration (`resumeLastState`).
* **Conditioned, Parameterized & Rising-Edge Transitions:** Direct evaluation of transition guards (`updateGuard`), rising-edge `trigger` switches, and parameterized transitions with state hierarchy ancestry traversal.

---

## 3. Streaming Dataflow, Buffering & Multi-Rate Processing

* **First-Class `buffer` Entities & Lookup Tables (`Buffer::ModeTable`):** Native sliding and circular ring buffers with automatic mirror allocation for boundary-safe, zero-copy frame reads (`Buf.frame`) into downstream reactions and loops, alongside unified `Buffer::ModeTable` lookup tables (static, reactive, and interpolated modes) and `RateAdapter` entities.
* **Transparent Multi-Rate Streams:** Topological rate staging resolves multi-rate stream chains during compilation. Downsampling decimation branches and upsampling execution loops are generated automatically without manual phase-timer math.
* **Dynamic Sizing & Automatic Broadcasting:** Output ports dynamically adapt to consumer dimensions, and array connections automatically lower to scalar input ports via broadcast semantics.
* **Lifecycle Stream Hooks & Inferred Persistence:** Native `onLoad` (one-shot per program execution), `onInit` (per domain instance), and `onReset` stream hooks, plus automatic persistence inference for internal module state without requiring explicit `persistent` declarations.

---

## 4. Multi-Domain Computation, Hardware Specialization & Embedded Targets

* **Multi-Domain Execution & Subdomains:** Parent-ticked and autonomous subdomain scheduling (`preSubdomains`, `postSubdomains`, `trigger`), inline domain invocations in streams, cross-domain shared memory signal validation, parameterized `connection` blueprints (`sourceEntities` and `destinationEntities`), and `Sleep` primitives across JIT, subdomains, and bare-metal targets.
* **Layered Hardware Overlay Hierarchy (`Board` → `Device` → `Core`):** Declarative hardware overlays resolved via `HardwareResolver`, including full **AVR** (`AVR5` core, `ATmega328P` device, and `ArduinoUno` board) target lowering, C runtime support, and framework-selected `Real` / `Int` type overlays.
* **Hardware-Specific Algorithmic Variants & Quality Policies:** Automated compile-time resolution for platform variants (e.g., ROM table-driven math, polynomial approximation, or hardware intrinsics) tagged with `@Device` or `@Policy` metadata.

---

## 5. Modular Standard Library, Typing, Schemas & Zero-Dependency Parsers

* **Modular Standard Library Packages (`import`):** Organized the standard library into canonical namespace packages (`Dsp`, `Math`, `IO`, `Control`, `Sensors`, `Estimation`, `Window`, `LinAlg`, `Testing`, and `Random`) with transitive and aliased (`import ... as ...`) package resolution.
* **Centralized Two-Phase Schema Enforcement:** Declarative validation rules in `Schema.stride` enforcing collection constraints (`contains`, `uniqueBy`), numeric boundaries (`multipleOf`, `allowedValues`), type finality (`final`), platform purity (`pure`), and visibility/mutability (`public`, `writeable`).
* **Lossy Conversion Auditing:** Compile-time warnings (`-Wconversion`) for narrowing numeric conversions and scalar-to-array calls, with automatic typecast resolution across stream elements.
* **Standalone Header-Only Parsers:** `StrideReader` and `StrideDocument` (`stride_data.h`) provide fast data parsing and validation without requiring the full compiler runtime.

---

## 6. Compiler Engine, State Safety, Runtime & Debugging

* **Source-Level DWARF Debugging:** Native line-by-line debugging emitted with DWARF metadata for step-in and step-over inspection in GDB and LLDB.
* **Zero Global Pollution:** Re-entrant, stack-allocated `__state` structs with exact domain matching and properly sized persistent array fields eliminate global state pollution for nested callables and multi-rate accumulators.
* **Modular Runtime (`StrideRuntime`):** Dedicated static runtime library with owned `String` storage, stack-allocated string result scratch buffers (`resultSize`), zero-copy `StrideStringView`, format strings, variadic concatenation, host-replaceable random number generation (`RandomInt`, `RandomChance`), and target sleep hooks.
* **Multi-Version Isolated Caching:** Disk-based AST and compiled object caching partitioned by compiler build hash (`b_<hash>`), with automated stale-version pruning, cross-target/board cache tag isolation, and automatic cache bypass on experimental (uncommitted) compiler builds.

---

## 7. Toolchain, Cross-Platform Installers, Build Systems & Verification

* **Unified `stride` & `stridec` CLI Toolchain:** Unified `stride` driver (`stride compile`, `stride build`, `stride flash`, `stride doctor`, `stride system`, `stride tool`, `stride setup vscode`) and Clang-compatible `stridec` AOT compiler supporting multi-file compilation, optimization levels (`-O0` to `-O3`, `-Os`, `-Oz`), feature flags (`-D`, `--feature`, `--no-feature`, `--no-default-features`), board firmware linking/uploading (`--boardOverride`, `--upload`, `--port`), and automated C header generation (`--emit-header`).
* **Cross-Platform Installer Packaging (CPack & CI/CD):** Automated GitHub Actions release pipeline producing Windows NSIS installers (`.exe` with automatic `PATH`, `STRIDEROOT`, and bundled VS Code `.vsix` extension installation) and portable `.zip` archives, Linux `.deb` and `.tar.gz` packages, and macOS `.tar.gz` and `.zip` bundles.
* **Native Build System Rules:** First-class support for **CMake** (`find_package(Stride)`, `stride_target_sources`, `stride_add_library`) and **Bazel** (`rules_stride` `0.1.0`), alongside precompiled Arduino ARM library bundle generation (`--emit-arduino-lib`).
* **Differential DSP Testing:** `stridetest` test runner and `Testing` standard library package featuring statistical signal verification (`ExpectSnr`, `ExpectMaxError`) alongside latency, jitter, and steady-state zero-heap allocation tracking.
