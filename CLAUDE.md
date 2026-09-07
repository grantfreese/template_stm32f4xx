# <PROJECT_NAME> AI Coding Instructions

Template note: derived from the harvest-bms-fw instructions. When instantiating a project from this
template, replace every `<PLACEHOLDER>`, resolve the TODO markers, and delete this note.

## Project Overview
<PROJECT_NAME> is an embedded firmware project for <PROJECT_DESCRIPTION>, built with modern C++20.

### Architecture
- **Build System**: CMake with Ninja generator
- **Target**: STM32F373 (Cortex-M4), cross-compiled with arm-none-eabi toolchain
- **RTOS**: FreeRTOS
- **Toolchain file**: `toolchain/arm-gcc-toolchain.cmake`
- **Output**: `build/bin/app.elf`

### License
This project is licensed under the GNU General Public License, version 2 or (at the recipient's
option) version 3. The license texts are `LICENSE_GPLv2.md` and `LICENSE_GPLv3.md` (verbatim —
never edit them). The version choice lives in the grant notice below, not in the license texts.

Grant notice (README and distribution points):

    This program is free software; you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation; either version 2
    of the License, or (at your option) version 3.

Each source file carries the license as a single SPDX line in its header
(`// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only`) — no multi-line license
boilerplate in files.

### Intentional terminology

When requirements define a term (marked with a NOTE), use that exact term in code, comments,
commits, and new requirements. No paraphrasing or synonyms.

**Hardware component names**: always full — code, comments, strings, logs, CLI. Write the full part
number ("LTC6813", not "LTC"; "ADG728", not "ADG").

**STM32 CAN peripheral**: call it "the CAN peripheral" or "CAN" — not "bxCAN".

## Code Style & Formatting

### Formatting (`.clang-format`)
- **Format-only commits**: run clang-format as a separate commit before behavior changes; never mix.
- **Run before commits**: `find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i`
- **Base**: Google style with Allman braces (braces on new line)
- **Line length**: 120 characters max
- **Indentation**: 4 spaces
- **Pointer alignment**: Left (`int* ptr` not `int *ptr`)
- **Parameters**: No bin-packing (one per line for readability)
- **Always use braces**: Use braces for if/else/for/while even with single-line bodies
- **No ternary operators**: Use if/else instead of `condition ? a : b`
- **Reduce dispatch chains**: matching-and-calling if-else ladders → helpers + short-circuit `||`.

### Naming Conventions (`.clang-tidy`)
Strict naming enforced via clang-tidy:
- **Classes/Enums/TypeAliases**: `CamelCase`
- **Functions/Methods**: `CamelCase`
- **Variables/Parameters**: `lower_case`
- **Class members**: `lower_case` with trailing underscore (`member_`), public and private
- **Constants**: `CamelCase` with `k` prefix (`kMaxSize`, `kDefaultValue`)
- **Macros**: `UPPER_CASE`
- **File names**: `lower_case` (e.g. `adc_monitor.cpp`, `spi_driver.h`)

### Comments
- **No banner comments**: no decorative divider lines around section headings; plain `//` only.
- **Don't narrate language mechanics or file layout.** "Declaration is in main.h",
  "Public so the test can reach it" -- a reader already knows how C++ works.
- **Don't redirect.** "See foo.h for full docs" wastes the reader and lets the
  two locations drift. Doc the declaration, leave the definition bare.
- **Don't explain code history.** Don't write what the code used to do, what an old
  version got wrong, or what would happen without this check. The current code stands
  on its own; history belongs in commit messages and PR descriptions. Exception: when
  the code is a workaround for an external constraint (a hardware quirk, a vendor bug,
  a spec deviation) and the historical context is genuinely needed to understand the
  workaround, write the minimum that makes it intelligible.
- **No non-ubiquitous acronyms or abbreviations.** Write `kOutputShutdown`, not `OS`;
  `BenchPsu`, not `BP`. Apply to comments, log lines, CLI output, commit messages, and
  ticket text. Industry-standard acronyms that any embedded engineer reads at a glance
  (CAN, UART, SPI, I2C, ADC, DMA, ISR, NVM, RTOS, GPIO, PSU) are fine. Project-internal
  coinages and state-name initials are not -- they save a few keystrokes for the writer
  and cost every reader a trip back to the legend.
- **No second-person ("you", "your") in comments, doc, error messages, CLI help, or debug output.**
  Write impersonally. "X returns the buffer size", not "you can call X to get the buffer size".
- **Section-label comments above multi-line blocks are scannability aids — keep them.** Short
  labels like `// reset faults` over a 4-line sequence are intentionally hand-written. The
  no-WHAT-comments rule targets restatements of a single adjacent line, not section headers.
- **"Shorten" or "revise" comments = trim to one terse phrase, not delete.** Delete only when
  the comment is pure restatement of the next line. If the comment captures a non-obvious WHY
  (constraint, hidden coupling, hardware quirk), keep that WHY as one short line. File headers
  stay strictly 3 lines (filename+desc, copyright, SPDX license identifier).
- **When told to remove something, remove it completely.** If the user calls out a specific
  comment or code as bad, fix it fully the first pass — don't leave partial remnants.

### No ticket numbers in code or conversation
Tn / Bn references belong in commit messages and PR descriptions only.
Forbidden in source (printfs, log lines, comments, doc, any embedded string)
AND in reviews, status updates, and conversation about the codebase. Describe
behavior in plain language: "the rising-edge current-trip line", not "T91's
diagnostic". Ticket numbers rot when tickets are closed or renumbered.

### File headers
Every C/C++ source file starts with a three-line header:
1. A banner line: `// <filename> - <short description>`. Example: `// main.cpp - Main application entry point`.
2. A copyright line: `// copyright Grant Freese <year>`.
3. An SPDX license line: `// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only`.

### Documentation
Doxygen `//! @brief` on public APIs (functions, classes, non-obvious members), placed
immediately before the declaration. Tag with `@param`, `@return`, `@pre`.

- **`//!` is ONLY for Doxygen tag lines** (`@brief`, `@param`, `@return`, `@pre`). Never use
  `//!` (or `//!<`) for a plain descriptive comment -- use `//`. A line starting with `//!`
  must contain an `@tag`.
- **`@brief` is one short line.** No multi-line prose wrapped under it; if it needs a paragraph
  to explain, the explanation belongs in the commit message, not the header.
- **Don't explain code like a college student.** Comments are terse pointers to non-obvious WHY,
  not narration of what the code does. Default to no comment.

### Python style
- **No docstrings**: use `#` line comments for all Python commentary, including module-level
  descriptions and function documentation. Never use triple-quoted strings (`"""..."""`) as
  comments or docstrings.

### CLI file organization
- **Subsystem-coupled** (one driver/task): `<subsystem>_cli.cpp` next to the subsystem
  (`src/tasks/task_<name>_cli.cpp`, `driver/<chip>/<chip>_cli.cpp`).
- **Cross-cutting** (status, fault, reset): `cli_<name>.cpp` in `src/cli/`.

## Modern C++ Practices
- **Write C++, not C**: encapsulate state in classes, use RAII, prefer the STL. Plain C style
  (file-scope statics, free functions standing in for methods, `void*` context pointers) is only
  acceptable when interfacing with a C API that cannot be wrapped.
- **C++ Core Guidelines**: All code must adhere to the [C++ Core Guidelines](doc/CppCoreGuidelines.md).
  Where project-specific rules in this file conflict with the guidelines, the project-specific rules take
  precedence.
- **Encapsulate state in classes**: if multiple functions pass the same args (folder + filename),
  those args are class state. Use read-only accessors, not exposed fields.
- **Prefer command-line arguments over environment variables** for program configuration, in both the
  main application and test executables.
- **Stack over heap**: Use stack allocation by default; heap only for large objects or when ownership
  semantics require it. Avoid heap-allocating containers (`std::unordered_map`, `std::map`,
  `std::function`) in hot paths such as ISRs or tight control loops. Prefer `std::array` with
  linear/binary search, `constexpr` lookup tables, or simple helper functions instead.
- **Brace initialization**: Always use brace form: `bool foo{true}`, `int x{0}`.
- **Avoid C preprocessor**: Only use `#define` when there's no other way (e.g., platform-specific code).
  Prefer `constexpr` variables over `#define` constants. **Never use `#ifdef __arm__` or similar
  guards to split host vs ARM compilation of a single .cpp** — split the file: pure-logic helpers
  in one .cpp (host + firmware), HAL/register code in another (firmware only), wired through CMake.
- **Use `#pragma once`** for include guards.
- **Prefer `std::string`** over `const char*` unless interfacing with C APIs.
- **Use standard library constants**: prefer `<numbers>` (e.g., `std::numbers::pi_v<float>`) over hardcoded
  values like `3.14159265358979323846f`.

### Building
Firmware (ARM cross-compile, produces `build/bin/app.elf`):
```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$(pwd)/toolchain/arm-gcc-toolchain.cmake
ninja -C build
```

Host tools + tests — single CMake project under `tools/` using the host toolchain:
```bash
cmake -B build-tools -S tools -G Ninja
ninja -C build-tools
```

**Monitor RAM/FLASH each build.** The firmware build prints `Memory region Used Size ... %age
Used` at the end — read it every time. Any RAM-heavy change (buffers, filters, stacks, FreeRTOS
objects, new globals) can shrink headroom fast. Flag integer-% crossings in the turn; at 88%+
proactively audit (`arm-none-eabi-nm --size-sort --print-size` for top BSS symbols).

If configure fails after moving or renaming the project directory, delete the stale cache first:
```bash
rm -rf build && cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$(pwd)/toolchain/arm-gcc-toolchain.cmake
rm -rf build-tools && cmake -B build-tools -S tools -G Ninja
```

**Flashing**: `scripts/flash.py` — programs `build/bin/app.elf` over SWD through a SEGGER J-Link
via JLinkExe and `CommandFile.jlink` (device STM32F373CC), then resets the MCU so the new firmware
boots. Transcript is suppressed on success; `-v` shows it. No power-sequencing constraints.

**Serial console**: TODO — document the console port, baud rate, and any udev symlink once bench
hardware is defined. Address USB-serial adapters by serial-number udev symlink, not the bare
`/dev/ttyUSBn` index (not stable when multiple adapters are plugged in).

**Prefer running tools through the build system**: When a tool has a CMake target, use
`cmake --build build --target <target>` rather than invoking the script directly.

System packages: `doc/install-packages.sh` (apt + python). Update it when adding a new system dependency.

### Peripheral/Driver Development
- Peripheral drivers live under `driver/` and are compiled as separate CMake targets
- Use HAL functions for register access rather than direct register manipulation where possible
- Keep ISR handlers minimal: set a flag or post to a FreeRTOS queue, do work in a task
- Follow existing driver patterns under `driver/` for class structure and SPI/I2C
  transaction sequencing
- Initialization order matters: ensure clocks and GPIO are configured before peripheral init
- **Never change pin assignments without user approval**: If a pin specified in a ticket or
  requirement is unusable, STOP and report the problem. Pin selection is a hardware design decision.
- **No parallel members for indexed state**: When the same piece of state exists for multiple
  instances of the same thing (chips, channels, sensors, buses), make it an array — `T foo[kN]`
  indexed by instance, not `foo` + `foo_2` or `foo_primary` + `foo_secondary`. A named copy
  hardcodes a count of two and forces every downstream function to duplicate its logic.
- **Class access sections**: use a single `public:` block followed by a single `private:` block.
  If a public member depends on a private member for initialization (e.g., holds a reference),
  move the dependency into the public section so the declaration order is correct — C++
  initializes members in declaration order, not by access specifier.

### CAN DBC
If the project broadcasts CAN messages, keep a DBC file in the project root and update it whenever
messages are added or modified — new message IDs, signal additions, payload layout changes, and
scaling changes must all be reflected in the DBC.

### Adding New Features
1. Consider interrupt and task safety: data shared between ISRs and tasks must use
   appropriate synchronization (queues, mutexes, critical sections)
2. Consider stack constraints: each FreeRTOS task has a fixed stack; avoid large local arrays
3. Consider timing requirements: identify whether the feature has hard real-time deadlines
4. Use stack allocation for small buffers; avoid heap allocation in recurring paths

## Development practices

### Don't stop mid-task without a blocker
Given a clear directive, work continuously to a real blocker: (1) unanswered question that shapes
work, (2) destructive action needing approval, (3) task complete. Long responses, finished chunks,
wanting acknowledgement — not blockers. Commit at natural stopping points, then continue.

### Don't guess about hardware configuration
Check actual bench capabilities (drivers, HIL helpers, existing tests) before claiming something
needs a mock or can't be driven. HIL failures under a realistic bench load are firmware bugs, not
test artifacts.

### Don't ask for data observable through tools
Read state directly: the firmware CLI, debugger (JLinkGDBServer + arm-none-eabi-gdb for
register/memory), files/git log for source state. Only ask the user for things not observable
from tools: physical bench wiring, hardware setup, intent, design decisions.

### Close every background job before ending a turn
Stop every Bash `run_in_background`, every Monitor, every long-running shell process before
ending the turn — never trust self-exit conditions. Use TaskStop on Monitors and `kill`/`pkill`
on backgrounded Bash. Hardware (serial port, GPIB) held across an end-of-turn handoff blocks
the user from interactive work.

### Code quality
- **Code expresses intent**: do exactly what it appears to do. No side effects piggybacking on
  unrelated operations — name the intent directly.
- **A function delivers its contract in one call.** Repeated identical calls = lying abstraction;
  fix the function or introduce a new one whose name honestly describes its behavior.
- **Refactor proactively** when files grow large or mix concerns. One primary class/concern per
  file. Do refactors on a separate branch, tests passing before and after.
- **Virtual dispatch over type-switch.** `dynamic_cast` / if-else chains over subclasses → virtual
  method on the base; new subclasses then require the override automatically.
- **Type-specific behavior belongs in the type**, not in a central dispatcher.
- **File size limit ~500 lines.** Past that, refactor — extract helpers, move type-specific
  logic into the types, or split along section boundaries.
- **New-subclass checklist**: search for `dynamic_cast<ExistingSubclass*>` and type-switches over
  the type. Refactor to virtual dispatch before adding the new subclass.
- **No raw pointers across task/ISR boundaries.** Use FreeRTOS queues (by value), mutex-protected
  access, or explicit ownership transfer.

### Code duplication
- Duplication report: `cmake --build build --target duplo-report` (outputs `duplo-output.txt`
  for AI, `duplo-report.html` for humans; min block 6 lines).
- Before writing new code, search for similar logic; parameterize blocks that differ only in a
  name/constant. HIL tests share `tests/hil/` headers.
- For refactoring tickets: pick top targets from `duplo-output.txt`. One chunk at a time, test
  and commit. If unsure and nothing is broken, leave it.

### Requirements
- Project requirements: `./doc/requirements.md`. Names use camelCase (e.g., `userInput.mouse`).

**File format** (not markdown sections): a plain-text title line, then the status-marker legend as
`#`-comment lines, then top-level `> group` lines (no marker). Each requirement is a 4-space-indented
child line `> [marker] group.child (Tnn)` with its ticket IDs in parentheses, followed by
8-space-indented `- ` detail bullets stating concrete, testable behavior (interfaces, pins,
addresses, defaults). Term definitions appear as `- NOTE:` bullets per the intentional-terminology
rules.

**Status markers:**
- `[ ]` - Not started, no ticket written
- `[~]` - Ticket written (include ticket ID, e.g., "(T16)" or "(B3)")
- `[?]` - Implemented but not yet tested
- `[x]` - Implemented and tested

When creating a ticket, change `[ ]` to `[~]` and add the ticket ID for all requirements covered.
For follow-up work on an already-implemented requirement, add the new ticket ID alongside existing
ones (e.g., "(T5)" becomes "(T5, T12)").

**Implementing requirements:**
- ONLY implement requirements explicitly specified by the user
- DO NOT implement parent, child, or sibling requirements unless explicitly requested
- If a requirement has child requirements, ASK which to implement before proceeding
- **Confirm instance scope before implementing**: When a feature applies to multiple parallel
  instances (multiple buses, sensors, channels, peripherals), explicitly list every instance that
  will be covered before starting. Do not silently collapse separate instances.

### Task management
Task tracking files live in the project root: `TASKS.md` and `ISSUES.md`.
- **TASKS.md** - One-line summaries grouped by status (Bugs, To-Do, In-Progress, Review, Done,
  Canceled). Each entry: `Bnn`/`Tnn` + brief description, nothing else. No comment tails, status
  hints, implementation details, dependency parens, or cross-refs. Full detail goes in ISSUES.md.
- **Ticket title length**: TASKS.md entries and the matching ISSUES.md `##` titles
  MUST be under 100 characters total (including the `Bnn:` / `Tnn:` prefix). No
  exceptions. Keep titles as short as practical -- a noun phrase, not a sentence.
  Bad: `- B63: Persist pack SoC to NVM with 5-min rate-limited writes, shutdown flush, and immediate end-of-charge write`.
  Good: `- B63: Persist SoC to NVM`. The why / how lives in ISSUES.md's
  Description section, not in the title.
- **ISSUES.md** - Full details, ordered newest-to-oldest

Task structure:
- **Feature tasks**: 'T' prefix (T1, T2). **Bug tickets**: 'B' prefix (B1, B2). IDs increment
  independently.
- Statuses: `to-do`, `in-progress`, `review`, `done`, `canceled`.
- TASKS.md has a separate **## Bugs** section above **## To-Do** for bug tickets in the to-do
  state. Once a bug moves to in-progress or later, it uses the same status sections as features.
- Task order (except `review`/`done`/`canceled`) indicates priority.
- Wrap text at 120 columns of content (not counting indentation). Applies to requirements.md,
  TASKS.md, ISSUES.md, and CLAUDE.md.

**File synchronization**: ALWAYS update TASKS.md and ISSUES.md together. When creating a task,
changing status, or moving to review/done/canceled, update both files in the same edit. Also
update requirements.md when creating a ticket.

**Moving a ticket between status sections in TASKS.md**: remove the line from the old section
in the SAME edit that adds it to the new one. Adding to In-Progress without deleting from To-Do
produces duplicate entries; verify each ticket appears exactly once after the edit.

**Issue content**: Each issue in ISSUES.md should include a **Testing** section describing how
the feature will be tested (manual steps, new HIL tests), which existing tests cover it, and
test considerations (edge cases, hardware dependencies).

**Bug tickets:**
- Bug issues in ISSUES.md include a **User Description** section at the top with the original
  user report, then standard fields (Description, Implementation Details, Acceptance Criteria,
  Testing). Claude populates the standard fields when starting work on the bug.
- **Priority**: Prefer open bug tickets over feature to-do items unless the user directs otherwise.
- **Discovering bugs during other work**: Create a bug ticket rather than fixing inline, to avoid
  context exhaustion on tangential work. Exception: regressions introduced by the current ticket
  must be fixed before considering the ticket complete.

**Filing and starting tickets:**
- **Never file a new TASKS.md/ISSUES.md entry without explicit user direction.** Proposing one
  in conversation is fine; wait for an affirmative ("file it", "add a ticket") before editing.
- **Never start a ticket without explicit direction.** Creating a ticket is not starting it.
  No branches, no status change to `in-progress`, no implementation until told to start that
  specific ticket.
- **Create a branch before any work** — including analysis. Name: `<TASK_ID>_<SHORT_DESCRIPTION>`.
  No work on master without explicit direction.
- Placeholder ticket (empty Implementation Details / Acceptance Criteria / Testing): flesh those
  out in ISSUES.md **before** starting implementation.

**Status workflow:** `to-do` → `in-progress` → `review` → `done`. Tasks move to `review` when
implementation is complete and ready for manual review. **NEVER move a task to `done` without
explicit direction from the user.** Use `canceled` for abandoned tasks.

**Move to `review` automatically when implementation is complete** — code written, builds clean,
pushed to the ticket branch. Do this in the same turn as the final commit; don't wait to be
asked. Only the `done` transition needs explicit user approval.

**Verifying task status (stale ticket detection):** When asked if a task is done, check both
TASKS.md and ISSUES.md (they may be out of sync), then verify against the actual codebase. If
code exists but the ticket shows incomplete status, the ticket is stale — report the discrepancy.

### Git workflow

**Commit messages:**
- Break changes into separate commits based on topic/purpose
- DO NOT include AI attribution text (no "Generated with Claude Code", no "Co-Authored-By: Claude")
- Clear, concise, professional. ASCII characters only (no Unicode symbols).

**Committing and merging:**
- **Commit after every logical change** — do not accumulate uncommitted work. After each
  self-contained change (bug fix, feature addition, refactor, rename), build, verify clean,
  and commit before moving on.
- Fix all build warnings before committing
- Prefer regular merge commits (`git merge --no-ff`) over fast-forward so each ticket's merge
  point is visible in the log
- **`git commit --amend` + `git push --force-with-lease` is allowed on ticket branches** (fix
  typos, mangled commit messages). The no-amend rule applies only to master.

**Pushing and merging permissions:**
- Ticket branches (T1_, B2_, etc.) may ALWAYS be pushed without asking. Push every ticket branch
  whenever you stop work (end of turn, end of session) so the remote reflects local state.
  Use `git push -u` for new branches; `git push --force-with-lease` after a rebase.
- Master branch must NEVER be pushed. No exceptions, even when implicitly requested — confirm
  explicitly each time.
- Merging to master requires explicit user approval — ALWAYS ask before merging. Approval is
  per-merge: a yes for T100 last week is not a yes for T101 this week.
- **Never create or switch branches without explicit direction in the current turn.** Commit on
  whatever branch the user is currently on. Earlier-session context like "let's work on T71" is
  not standing authorization to switch back later.
- **Never revert, undo, or roll back changes without explicit user direction.** Acknowledging a
  mistake does not authorize undoing it — ask what to do, or wait.

### Managing related feature branches
- **Integration branches** (default for related features): When multiple tasks modify the same
  files or share a feature set, group them under one integration branch as separate commits, with
  a single rebase point and single merge to master.
- **Branch chaining** (for strict dependencies): When Ticket B cannot compile without Ticket A,
  branch B from A. Document with "Depends on: TA" in ISSUES.md. To merge: merge A to master
  first, then rebase B onto master, then merge B.
- **Independent features in different files**: Separate branches from master.

Before starting a new task, check ISSUES.md for in-progress work touching the same files.

### Unit tests
Simple executables (no framework), auto-discovered. Add via
`tests/unit/test.unit.<req-name>.cpp` (return 0 on success); declare firmware sources in
`test.unit.<req-name>_EXTRA_SOURCES` in `tests/CMakeLists.txt`; re-run cmake to discover.

TODO: document the test runner and coverage target for this project (this template does not
use `mm`).

### HIL (Hardware-in-Loop) tests
HIL tests run on connected hardware and verify end-to-end behavior across firmware, drivers,
and physical peripherals over JLink/SWD or serial.

- When a test fails, re-run to determine if it is flaky or broken
- NEVER skip tests without asking the user first
- Always verify test executable names in CMakeLists.txt before documenting how to run them
- **No fixed delays; timeouts are guards, not fixes.** Never `sleep` to wait for hardware —
  fixed delays are race conditions. Poll a status flag or use a signaling mechanism. A poll's
  timeout exists only so a broken firmware cannot hang the test forever; raising it to silence
  a flake masks the bug.
  - Short defaults: CLI ack well under 1 s; a 5 s timer gets 5 s + small margin.
  - Deliberately large timeout: comment must say why.
  - Intermittent failures are always a signal to investigate, never to inflate the timeout.
- **Never clear NVM or hand-calibrated bench values without per-instance approval.** Restore
  specific fields by name rather than resetting to defaults wholesale.
- **Mocks.** Default to real bench values; mock only what the bench can't physically produce.
  Unnecessary mocks slow tests, wear flash, and obscure intent. **New mock = per-instance
  approval** — first widen an existing mock if its semantics overlap; ticket authorization does
  not authorize new mocks.
- **Read CLI output line-by-line.** The CLI emits `\r\n`-terminated lines. Waiting on a line
  prefix and then searching the buffer for a later substring is flaky — the wait returns before
  the rest of the line streams in, leaving a truncated buffer. Wait for the complete line
  containing the expected text instead.

**Adding a new HIL test:**
1. Create `tests/hil/<descriptive_name>_test.cpp`
2. Register the executable in `tests/CMakeLists.txt`
3. Register the test with the test runner (details TBD as HIL infrastructure is established)

**Test terminology:**
- **Flaky test**: intermittently passes/fails without code changes (non-deterministic). Often
  timing, races, or hardware variability.
- **Broken/failing test**: consistently fails. Indicates a real bug in firmware or test.

### Test coverage policy
- **Exhaustive coverage, or a failing test.** Every branch, transition, and observable behavior
  must be tested. If a transition can't be exercised from the current interface, add a testable
  hook or delete the dead code — don't leave the gap silent. Partial coverage that claims PASS
  is worse than a missing test.
- Tests for derived classes must also test overridden methods from base classes
- Each class should have tests verifying its complete interface
- **HIL tests are required for hardware-facing features** — any feature involving peripheral
  communication (GPIO, I2C, SPI, ADC, CAN, etc.): driver read/write, protocol correctness
  (byte sequences, ACK/NACK), and hardware-event-triggered state transitions
- **Range tests must verify boundary conditions** — minimum, maximum, out-of-range, and
  nominal values. A single nominal sample is insufficient.

### Test naming convention
- HIL tests: `test.hil.<requirement-name>`
- Unit tests: `test.unit.<requirement-name>`
- Test names must correspond to existing requirement names. Never invent sibling requirement names.
- Test executable names match test names exactly.

### Test quality guidelines
- **Test assertions must match test claims** — if a test claims to verify "properly formatted
  file", the assertions must verify that claim, not just check loosely related conditions
- **Avoid weak verification** — prefer exact equality over substring containment when verifying
  output format or structure
- **Minimize verbose output** — print details only on failure; successful tests should be quiet
- **Tests should be deterministic** — verify exact expected output rather than partial matches

### sudo policy
NEVER use sudo without asking first. Explain what the command does and why it needs elevated
privileges before requesting approval.
