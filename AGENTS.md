# AGENTS.md

## Purpose

This repository builds **grafitt**, a single-header C++ graph toolkit inspired by OCamlGraph, with:

- immutable and mutable graph APIs
- builders for both graph styles
- Queryfitt graph query DSL
- graph rewriting
- GBIN binary serialization
- schema-driven loading via `*.sktl`
- compact, maintainable, header-first implementation

This file tells an AI coding agent how to work in this repository with good judgment, low token waste, and minimal churn.

---

## Primary Objectives

When modifying this project, prioritize:

1. **Correctness over breadth**
2. **Token economy over verbose exploration**
3. **Small diffs over sweeping rewrites**
4. **Header-only simplicity over framework sprawl**
5. **Compatibility with existing repository conventions**
6. **Deterministic, inspectable designs**
7. **Composable APIs modeled after OCamlGraph where appropriate**

---

## Working Style

### Do

- Read only the files directly relevant to the current task.
- Prefer surgical edits.
- Reuse existing types, helpers, naming patterns, and file layout.
- Preserve public API stability unless the task explicitly requires API changes.
- Keep generated code and hand-written code clearly separated.
- Summarize intent briefly before large edits.
- When uncertain, propose 2–3 concrete options instead of producing a large speculative rewrite.
- Keep examples short and executable in spirit.
- Prefer total, explicit code paths over clever magic.

### Do Not

- Do not rewrite unrelated sections “for consistency”.
- Do not add heavy dependencies.
- Do not introduce build-system complexity unless asked.
- Do not dump long design essays into source comments.
- Do not create speculative abstractions with no immediate use.
- Do not consume tokens restating obvious context repeatedly.
- Do not claim a file or API supports a feature without checking it first.
- Do not infer textual parser-combinator support from `DSLUtils.hpp` without verification.

---

## Repository Expectations

The codebase should converge toward these principles:

- **`grafitt.hpp`** is the main deliverable.
- Prefer **single-header or very low-fragmentation design**.
- Keep mutable and immutable graph interfaces clearly distinct.
- Support graph traversal, querying, rewriting, and serialization coherently.
- Make schema and DSL artifacts human-readable.
- Keep docs aligned with actual behavior.

---

## Model of the System

### 1. Graph Layer

Implement two main graph families:

- **Imperative / mutable**
- **Persistent / immutable**

Maintain a clean separation in API and semantics.

Expected operations include:

- vertex insertion/removal
- edge insertion/removal
- membership tests
- iteration over vertices and edges
- neighborhood queries
- folds/maps where appropriate
- builders for incremental construction

### 2. Query Layer

Queryfitt should support:

- node/edge pattern matching
- attribute predicates
- path queries
- traversal constraints
- reachability
- aggregation

There are two forms:

- **embedded C++ DSL**
- **textual DSL**

### 3. Rewriting Layer

Graph rewriting should support:

- match patterns
- bind variables
- test predicates
- construct replacements
- optionally serialize/deserialize pattern specs

### 4. Serialization Layer

GBIN and related schema-driven formats should support:

- graph persistence
- versioned fields where practical
- inspectable schema definitions
- stable encoding rules
- configurable schema lookup directory via environment variable

---

## Required Technical Direction

### Matcheroni

Use **Matcheroni** for PEG-style textual pattern recognition when working on:

- Queryfitt textual parsing
- textual rewrite pattern parsing
- other compact grammar-driven recognizers

Follow this rule:

- **Matcheroni matches structure**
- **semantic conversion happens afterward**

In other words:

1. use Matcheroni to recognize syntax and isolate spans
2. build AST / typed values in a separate pass

Do not overload matching code with semantic side effects unless there is a strong local reason.

### DSLUtils

Use **`DSLUtils.hpp`** primarily for:

- embedded C++ DSL construction
- AST composition
- operator-driven expression building
- rewrite/transformation helpers if present

Do **not** assume it is the correct tool for parsing external textual DSLs unless the file has been checked and that capability is explicitly verified.

### SerdeTk

Use **`SerdeTk.hpp`** for:

- serialization/deserialization glue
- structured interchange
- JSON/XML/YAML/S-expression-related conversion if supported by the existing code

Do not invent SerdeTk APIs. Check names before using them.

### SKTL / BSON / GBIN

When editing schema files:

- preserve consistency with existing `*.sktl` style
- prefer explicit field names
- include versioning hooks where useful
- keep binary layouts simple and deterministic

---

## Token Economy Rules

This section is mandatory behavior for GPT Codex.

### Core Rule

Before acting, reduce the task to the **smallest sufficient unit**.

### Read Strategy

- Read the minimum number of files needed.
- Skim first, then zoom into exact symbols/functions.
- Avoid rereading the same long file unless new evidence requires it.
- If a task mentions a symbol, search for that symbol specifically.

### Write Strategy

- Make the smallest patch that solves the problem.
- Prefer updating existing code over introducing new subsystems.
- Keep helper functions local if they are not reused.
- Avoid verbose comments unless they document a subtle invariant or file format.
- Keep markdown concise and structured.

### Reasoning Strategy

- Do not narrate every thought.
- Provide short decision summaries.
- If uncertainty remains, state it in one compact note.
- Offer bounded alternatives, not open-ended brainstorming.

### Output Strategy

- For large tasks, return results in batches.
- For multi-file work, group related edits together.
- Summarize:
  - what changed
  - why
  - any assumptions
  - next step

### Anti-Bloat Checklist

Before finalizing, ask:

- Can this be solved with fewer edits?
- Did I add any abstraction not yet needed?
- Did I repeat repository context unnecessarily?
- Did I verify file-local naming and API usage?
- Is there any safer smaller version of this patch?

If yes, shrink the patch.

---

## File-Handling Guidance

### When Editing `grafitt.hpp`

Prefer this order:

1. types and traits
2. storage model
3. graph primitives
4. builders
5. query AST / query execution
6. rewrite AST / rewrite execution
7. serialization hooks

Keep unrelated concerns separated by clearly labeled sections.

### When Editing `GBIN.sktl`

- Keep field ordering stable.
- Include explicit magic/version fields.
- Prefer compact but readable definitions.
- Document assumptions briefly in schema comments if needed.
- Respect the environment-variable-based schema search path.

Suggested environment variable:

- `GRAFITT_SCHEMA_DIR`

Fallback default:

- `specs/`

### When Editing `specs/Queryfitt.ebnf`

- Keep grammar readable first.
- Avoid ambiguous productions where possible.
- Separate lexical and syntactic conventions.
- Reflect actual parser behavior, not aspirational syntax.

### When Editing `specs/PatternMatching.schema.json`

- Keep schema strict enough to validate structure.
- Avoid over-constraining extensibility fields.
- Include definitions for:
  - match clauses
  - binds
  - predicates
  - replacements
  - metadata/versioning

### When Editing Manual Files

- Keep examples short.
- Prefer one concept per section.
- Make chapter structure stable.
- Avoid duplicating reference material across chapters.

---

## Decision Rules

When multiple solutions are possible, prefer the one that is:

1. simpler
2. more local
3. easier to inspect
4. less dependency-heavy
5. more consistent with OCamlGraph-inspired interfaces
6. easier to serialize and test

---

## Acceptable Change Pattern

A good agent change usually looks like this:

1. inspect exact files/symbols needed
2. confirm assumptions from code
3. patch only relevant sections
4. add or update a short example/test/doc stub if useful
5. summarize succinctly

A bad agent change usually looks like this:

1. broad speculative redesign
2. many new abstractions
3. unverified API usage
4. long commentary
5. unrelated cleanup

---

## Handling Uncertainty

If a required capability is unclear:

- verify it from the file first
- if still unclear, implement the narrowest safe fallback
- document the fallback briefly
- avoid pretending unsupported features exist

Examples:

- If `DSLUtils.hpp` does not clearly support textual parser combinators, use Matcheroni or a small dedicated parser layer instead.
- If SerdeTk function names are uncertain, isolate integration behind thin wrappers until verified.

---

## Documentation Policy

When adding docs:

- keep reference material crisp
- prefer bullet lists over essays
- include one minimal example per concept
- align terminology exactly with code
- avoid promising behavior not implemented yet

---

## Testing Mindset

Even if no test framework is present, think in terms of testable slices:

- empty graph
- singleton graph
- multivertex edge insertion
- self-loop handling
- duplicate edge policy
- immutable update returns new graph
- query parse success/failure
- rewrite application success/failure
- GBIN round-trip

If adding examples, prefer examples that exercise one invariant at a time.

---

## Patch Planning Template

Use this compact internal plan shape:

1. target files
2. exact symbols/sections
3. smallest viable change
4. risk/assumption
5. follow-up if needed

Do not expand beyond this unless the task truly requires it.

---

## Communication Format

When reporting progress, use this compact structure:

- **Changed:** file(s) and key sections
- **Why:** one short reason
- **Notes:** assumptions or caveats
- **Next:** optional immediate follow-up

Keep it short.

---

## Final Reminders

- Be accurate about library capabilities.
- Use Matcheroni for textual PEG matching.
- Use DSLUtils for embedded DSL mechanics unless verified otherwise.
- Keep `grafitt.hpp` coherent and compact.
- Preserve token budget by avoiding unnecessary reading, rewriting, and narration.
- Prefer small, correct, composable progress over ambitious but noisy output.
-


## Appendage: Regular Smoke Tests

Use this appendix as a lightweight checklist after any nontrivial edit.

### Goal

Catch obvious regressions early with minimal token and time cost.

Smoke tests are not exhaustive. They are quick checks for:

- compile breakage
- API drift
- gross parse failures
- obvious serialization errors
- basic immutable/mutable behavior regressions

---

## When to Run

Run smoke tests after changes to:

- `grafitt.hpp`
- query parsing or Queryfitt grammar
- rewrite logic
- `GBIN.sktl`
- serialization code
- public graph APIs
- examples in the manual

For doc-only wording changes, smoke tests can be skipped unless examples changed.

---

## Minimum Smoke Test Set

### 1. Header Inclusion Test

Verify the main header includes cleanly in a tiny program.

Checks:

- standalone include works
- no accidental dependency on hidden include order
- no obvious syntax break

Minimal shape:

```cpp
#include "grafitt.hpp"

int main() { return 0; }
```

---

### 2. Mutable Graph Basic Test

Create a small mutable graph and verify:

- vertex insertion works
- edge insertion works
- membership checks behave
- iteration sees expected elements
- edge removal works
- vertex removal updates adjacency correctly

Suggested case:

- vertices: `a`, `b`, `c`
- edges: `a->b`, `b->c`
- remove `b->c`
- confirm `a->b` remains

---

### 3. Immutable Graph Persistence Test

Create a persistent graph and verify updates return new values without mutating the old graph.

Checks:

- `g0` empty
- `g1 = add_vertex(g0, x)`
- `g2 = add_edge(g1, x, y)`
- `g0`, `g1`, `g2` remain distinguishable by content

This is the fastest way to catch accidental in-place mutation in persistent code.

---

### 4. Traversal/Iteration Test

Verify traversal APIs on a tiny graph:

- `iter_vertex`
- `fold_vertex`
- `iter_edges` or equivalent
- neighbor iteration

Checks:

- all inserted vertices are visited once
- inserted edges are seen
- fold counts match expectations

Prefer tiny deterministic graphs over large random ones.

---

### 5. Queryfitt Parse Smoke Test

For the textual DSL, verify:

- one valid query parses
- one invalid query fails cleanly
- variable binding syntax is recognized
- a simple predicate/path construct is accepted if implemented

Keep this test narrow. It should only detect obvious parser breakage.

Example categories:

- simple node match
- edge pattern
- path/reachability
- malformed token sequence

If Matcheroni tracing exists behind a debug switch, keep it off by default in smoke tests.

---

### 6. Embedded DSL Construction Test

Verify the C++ DSL still builds basic query expressions.

Checks:

- basic expression composition compiles
- operator overloads still resolve
- AST node creation still works
- no namespace/API drift

This test can be compile-only if execution setup is expensive.

---

### 7. Rewrite Rule Smoke Test

Create one tiny rewrite:

- match a simple graph fragment
- apply a replacement
- verify the graph changes as expected

Example shape:

- match `a -> b`
- rewrite to `a -> c` or annotate `b`

Checks:

- matching does not crash
- bound variables survive to replacement
- replacement graph operation is coherent

---

### 8. GBIN Round-Trip Test

Verify:

1. build a small graph
2. serialize to GBIN
3. deserialize
4. compare structure and key attributes

Checks:

- magic/version accepted
- node count preserved
- edge count preserved
- representative attributes preserved

Use the smallest nontrivial graph:
- at least 2 vertices
- at least 1 edge
- at least 1 attribute if supported

---

### 9. Schema Lookup Test

If schema loading depends on an environment variable, verify both paths:

- `GRAFITT_SCHEMA_DIR` set
- fallback default `specs/`

Checks:

- custom directory is honored
- missing env var falls back correctly
- failure message is short and actionable when schema is absent

---

### 10. Documentation Example Smoke Test

For any code snippet added to docs/manual:

- ensure names match the actual API
- ensure examples are not using removed helpers
- compile snippets when practical

A short compile-only pass over example fragments is often enough.

---

## Recommended Frequency

### Every meaningful code edit
Run at least:

- header inclusion
- one mutable/immutable graph test
- one parser or rewrite test if relevant

### Before merging a feature-sized change
Run the full minimum set.

### Before publishing documentation or release artifacts
Run:

- full smoke set
- doc example checks
- GBIN round-trip
- schema lookup test

---

## Fast Failure Rules

If a smoke test fails:

1. stop adding features
2. isolate the failure
3. fix the regression first
4. rerun the smallest relevant subset
5. then rerun the baseline minimum set

Do not stack new edits on top of a known failing smoke test unless the failure is intentionally transitional and clearly noted.

---

## Token-Efficient Smoke Test Policy for Agents

When acting as GPT Codex:

- propose the smallest smoke test set relevant to the patch
- do not design an elaborate test matrix unless asked
- prefer 1–2 focused tests per changed subsystem
- summarize expected outcomes briefly
- avoid verbose test scaffolding for straightforward checks

Good example:

- “Added a parse smoke test for valid/invalid Queryfitt input and a GBIN round-trip check for a 2-node graph.”

Bad example:

- generating a full property-based test plan for a two-line fix

---

## Suggested Directory Layout

If the repository later gains explicit test files, prefer a compact structure such as:

```text
tests/
  smoke/
    include.cpp
    mutable_basic.cpp
    immutable_persistence.cpp
    query_parse.cpp
    rewrite_basic.cpp
    gbin_roundtrip.cpp
```

Keep smoke tests small and readable.

---

## Smoke Test Acceptance Standard

A patch is in good smoke-test shape if:

- it compiles cleanly
- the changed subsystem has at least one direct check
- one neighboring subsystem check still passes
- no obvious public API/example drift remains

---

## Minimal Smoke Matrix

Use this compact matrix as a default:

| Change type | Minimum smoke tests |
|---|---|
| `grafitt.hpp` core graph edits | include + mutable basic + immutable persistence + traversal |
| Queryfitt textual parser | include + valid parse + invalid parse |
| embedded DSL changes | include + compile-only DSL construction |
| rewrite engine changes | include + rewrite basic + one parse/match check |
| GBIN/schema changes | include + GBIN round-trip + schema lookup |
| docs with code examples | include + compile example fragments |

---

## Closing Rule

Smoke tests should stay:

- short
- regular
- targeted
- cheap enough to run often

If they become slow or elaborate, split them into a deeper test tier and keep the smoke layer minimal.
