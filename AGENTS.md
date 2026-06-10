# AGENTS.md

## Purpose

This repository contains `Grafitt`, a production-oriented **header-only C++20 graph library** under the `grafitt` namespace, with submodules including `builder`, `algo`, `queryfitt`, `rewrite`, and `gbin`.

Codex should treat this project as a **serious library engineering task**, not a toy container of algorithms. Prefer principled API design, generic correctness, documentation quality, and subsystem coherence over one-off feature patches.

The current library already includes:
- mutable and persistent graph variants
- traversal/fold style APIs inspired by OCamlGraph
- rewrite infrastructure
- SerdeTk/GBIN-related serialization hooks

Before changing serialization, inspect what already exists and extend it carefully rather than duplicating it.

## Working Mode

Use **medium reasoning effort** by default.

Work iteratively:
1. inspect existing APIs and invariants
2. identify extension points
3. implement features in a style consistent with the existing header
4. add Doxygen-friendly documentation
5. add or update examples/tests if the repository has a testing style already

Do not invent a parallel architecture when the existing one can be extended cleanly.

## Core Expectations

When adding features, Codex should aim to **boost Grafitt into a production-ready header-only graph library**.

Prioritize:
- algorithmic correctness
- genericity across vertex and edge-label types
- compatibility with both persistent and imperative graph styles
- low-friction API ergonomics
- good asymptotic behavior
- careful handling of directed vs undirected invariants
- strong documentation coverage
- minimal unnecessary dependencies
- header-only friendliness

Preserve the existing style and namespace structure unless there is a compelling reason to refactor.

## Architectural Rules

### General
- Keep everything compatible with **C++20**.
- Keep the library **header-only**.
- Prefer extending existing namespaces like `grafitt::algo`, `grafitt::rewrite`, `grafitt::gbin`, etc.
- Reuse existing exception types such as graph/query/rewrite/serialization-specific errors where appropriate.
- Prefer templates, concepts, and constrained overloads over ad hoc runtime polymorphism.
- Preserve existing semantics for persistent vs imperative graph APIs.

### Graph invariants
- Respect the library’s directed/undirected behavior.
- Do not break edge symmetry handling for undirected graphs.
- New algorithms must clearly define behavior on:
  - directed graphs
  - undirected graphs
  - disconnected graphs
  - self-loops
  - multi-edges, if supported by the existing representation
  - negative weights, where applicable

### API design
- Prefer composable free functions in `grafitt::algo` for algorithms.
- Add helper structs/types only when they materially improve clarity or extensibility.
- Keep naming consistent with existing OCamlGraph-inspired conventions where possible.
- Prefer return types like `std::optional`, result structs, or rich value objects instead of ambiguous sentinel values.

## Major Feature Goals

Codex should implement and/or improve the following, while inspecting the existing code first to avoid duplication.

### 1. Traversal and ordering algorithms
Add or strengthen:
- depth-first traversal variants
- breadth-first traversal variants
- topological sort
- stable topological ordering utilities where meaningful
- cycle detection helpers tied to topological sorting failure
- strongly connected components
- weakly connected components
- condensation graph utilities if the architecture supports them cleanly

If topological sorting already exists, improve API quality, diagnostics, and documentation.

### 2. Shortest-path and pathfinding algorithms
Add a more modern and complete shortest-distance toolkit.

Implement or improve:
- Dijkstra for non-negative weighted graphs
- Bellman-Ford for graphs with negative edges
- negative cycle detection
- **negative cycle extraction** returning the actual cycle, not just a boolean
- all-pairs shortest paths only if it fits the current architecture cleanly
- path reconstruction utilities
- shortest-distance-only APIs when the path itself is not required
- weighted shortest path abstractions using user-provided weight functions

Also add:
- **incremental shortest path** support where feasible and well-scoped

For incremental shortest path:
- prefer a clearly documented, practically useful design
- support updates such as edge insertion, edge deletion, or edge weight change if the data model allows it
- if fully dynamic maintenance is too invasive, implement a well-designed incremental/decremental subset with explicit guarantees
- document complexity and invalidation behavior carefully

Do not fake “incremental” behavior with unclear semantics; be explicit about what updates are supported.

### 3. Sorting and structural graph algorithms
Add or improve:
- topological sort
- transitive closure/reachability helpers where useful
- spanning tree / forest helpers if they fit naturally
- bridge/articulation-point algorithms if consistent with the graph model
- bipartite checking where natural

### 4. Serialization and persistence
The library already uses SerdeTk-related facilities. Do **not** re-add generic SerdeTk integration from scratch.

Instead:
- inspect existing serialization support first
- determine whether serialization/deserialization to **JSON**, **XML**, **YAML**, and **S-expression** formats already exists
- if any of those formats are missing, add them in a style consistent with what the current SerdeTk integration already supports
- strengthen and modernize **GBIN** serialization
- improve round-trip reliability
- improve error handling and diagnostics
- document schema/format assumptions
- preserve backwards compatibility when feasible

If serialization formats depend on what SerdeTk actually offers in this repository, inspect SerdeTk and adapt to its real APIs rather than guessing.

### 5. Visualization/export
Add graph rendering/export support for:
- **GraphViz DOT**
- **LaTeX TikZ**
- **Graph.js**

Design these as clean export/serialization-style APIs.
Prefer:
- configurable label rendering
- controllable styling hooks
- safe defaults
- separation between graph data and presentation options

The visualization falls under `grafitt::vizz` namespace.

### 6. Rewrite system improvements
Treat the rewrite subsystem as first-class.

Inspect the existing `grafitt::rewrite` facilities and then:
- strengthen the rewrite engine
- improve rule representation and matching ergonomics
- improve diagnostics and failure reporting
- improve composability of rewrites
- improve determinism or strategy control where possible
- document invariants and intended usage clearly

If there is already a rewrite pipeline, extend it instead of replacing it casually.

### 7. E-graphs and equality saturation
Add an e-graph subsystem if one does not already exist in meaningful form.

Goals:
- e-class / e-node representation
- congruence maintenance
- equality saturation workflow
- extraction of best terms/representatives under a cost model
- integration with the existing rewrite engine so rewrite rules can feed equality saturation
- mechanisms to apply rewrite findings back into graph-oriented structures where sensible

This should not be a superficial toy implementation. Prefer a minimal but principled design that can grow.

All the e-graph facilities fall under the `grafitt::eqsat` namespace. Both matching, and rewriting. 

### 8. Graph isomorphism / equality
Add graph comparison facilities, including:
- structural equality where exact vertex identity matters
- graph isomorphism checks where vertex renaming is allowed
- configurable equality/isomorphism notions if labels or directionality matter
- canonicalization helpers if useful

Document complexity and intended graph classes clearly. If exact general graph isomorphism is too heavy for a first pass, provide a well-documented staged approach:
- fast rejects / invariants
- canonical labeling heuristics
- exact checks where implemented


### 9. Graph covering and decomposition
Add a serious subsystem for **graph covering / decomposition**, aimed at breaking large graphs into smaller, algorithmically useful pieces such as **DAGs**, **forests**, **trees**, or other structured subgraphs.

Possible goals include:
- covering a graph with smaller DAGs
- covering a graph with trees or forests
- edge covers or vertex covers where appropriate
- decompositions that make downstream traversal, optimization, rewriting, or analysis easier
- heuristics and exact algorithms where each is appropriate and clearly documented

Codex should inspect the current graph model first and then add APIs for:
- decomposition result types
- cover construction algorithms
- reconstruction or provenance metadata showing how the cover relates to the original graph
- validation helpers to check whether a proposed cover is valid
- cost/quality metrics for comparing covers

If multiple notions of “cover” are implemented, document them clearly and keep them separate in the API.

For DAG/tree covering in particular:
- define precisely whether the cover is by vertices, edges, or both
- define whether overlap is allowed
- define whether the objective is minimizing number of pieces, balancing size, preserving labels, preserving reachability, or enabling faster downstream algorithms
- provide practical heuristics if exact optimization is too expensive
- expose enough metadata to map results back onto the original graph

This subsystem should integrate well with:
- traversal/pathfinding algorithms
- graph visualization/export
- graph rewriting
- equality saturation / e-graphs where meaningful
- serialization of decomposition results

### 10. Pattern matching on graphs
Add or strengthen **graph matching** facilities.

This should include, where appropriate:
- structural pattern matching
- subgraph matching
- labeled matching
- directed/undirected-aware matching
- exact and heuristic matching paths where appropriate
- APIs for match results, bindings, and witnesses

If the library already has matcher or rewrite-related infrastructure, reuse it rather than creating a disconnected matching system.

Graph matching should integrate with:
- rewrite rules
- query facilities
- graph isomorphism/equality checks
- covering/decomposition workflows

Prefer a design where patterns, matches, and rewrites can compose cleanly.

If general subgraph isomorphism is implemented:
- document complexity honestly
- provide fast prefilters/rejects
- expose practical usage modes for smaller graphs or constrained pattern classes

If only partial matching support is feasible in the first pass, implement it in a principled way and document the intended expansion path.

You may also use *Matcheroni* library, that is in the root of the project.

### 11. Matching via DSLUtils
`DSLUtils.hpp` already provides matching and rewrite capabilities. Codex should inspect and reuse them instead of inventing a disconnected matcher subsystem.

Relevant facilities in `DSLUtils.hpp` include:
- `dsl::pattern<S>` for compile-time string/tag patterns (`DSLUtils.hpp:629`)
- `pattern<S>::matches(std::string_view)` for anchored lightweight pattern checks (`DSLUtils.hpp:660`)
- `dsl::RewriteRule<Tag, Pred, Trans>` for rule-based rewrites (`DSLUtils.hpp:1203`)
- `dsl::rule<Tag>(pred, trans)` as a rule factory (`DSLUtils.hpp:1242`)
- `dsl::RewriteSet<Rules...>` for bottom-up repeated rewriting to fixpoint (`DSLUtils.hpp:1258`)
- `dsl::Rewritable<T>` as a capability concept for rewrite-enabled DSL types (`DSLUtils.hpp:2576`)

Codex should add matching features that integrate Grafitt with these capabilities, especially for:
- query/tag dispatch
- graph pattern selection
- rewrite rule application
- e-graph rule feeding
- matcher-backed graph decomposition and covering workflows

For string/tag-based matching:
- reuse `dsl::pattern<S>` and `when`/`match` style dispatch rather than adding a second mini-pattern language
- use it for lightweight matching of query terms, node kinds, labels, rewrite tags, or grammar-like graph DSL constructs
- document the supported pattern syntax and its limits instead of implying full regex support

For rewrite-aware matching:
- align Grafitt’s rewrite layer with `dsl::RewriteRule` and `dsl::RewriteSet` where the representations are compatible
- preserve clear boundaries between graph matching, AST matching, and graph rewriting, but make them interoperate cleanly
- use `dsl::Rewritable` where capability gating is helpful

For graph matching APIs:
- expose result objects with bindings, witnesses, or matched substructures
- support exact structural matching first, then extend toward subgraph matching where feasible
- integrate matching with graph isomorphism/equality, query execution, and rewrite pipelines

Do not treat `DSLUtils.hpp` as a generic regex engine. Its matching support is lightweight and intentional, so integration should respect its actual semantics and strengths.

### 12. Plugin system and module ABI
Add a serious **plugin/module system** for Grafitt based on **C99-compatible shared-library modules** with a **strict, versioned ABI** defined in `Grafitt-Module.h`.

This subsystem should be designed as a production-quality extension mechanism, not a casual dynamic-loading experiment.

#### Core model
Each plugin module should be a shared library loaded by the Grafitt host through **Dynalo** or an equivalent thin dynamic-loader abstraction already present in the project.

The host remains fully responsible for:
- discovery
- load order
- symbol resolution
- compatibility checks
- initialization
- dependency ordering
- registration
- teardown
- error isolation and diagnostics

Plugins must not assume they own global process state or host lifecycle.

#### Required exported symbols
Each module should export a small, stable C ABI surface, at minimum:
- `GM_ModMetadata`
- `GM_ModStart`
- `GM_ModEnd`

These exports must use C linkage and ABI-stable types only.

`GM_ModMetadata` should provide enough information to reject incompatible or unsafe modules before startup. At minimum it should declare:
- ABI version
- module name
- module version
- capability flags

Codex should strengthen this metadata design so it also supports, where appropriate:
- minimum supported host ABI
- maximum supported host ABI, if version windows are useful
- module build identifier
- module vendor/author string
- feature flags
- dependency declarations
- optional capability/version subrecords
- declared thread-safety level
- license/notice metadata if the project wants redistributable module inventories

#### ABI rules
The plugin boundary must be **strictly C ABI oriented**:
- no C++ types across the ABI boundary
- no STL containers across the ABI boundary
- no exceptions crossing the module boundary
- no ownership ambiguity
- no inline-template ABI coupling between host and module
- no reliance on compiler-specific C++ name mangling

All cross-boundary structs should use:
- fixed-width integer types
- explicit sizes/versions
- documented ownership rules
- reserved fields for ABI evolution
- explicit nullability semantics

Prefer ABI patterns such as:
- versioned structs with `size` fields
- function tables / vtables for extensibility
- opaque handles instead of exposed internal state
- explicit status/error codes instead of exceptions

#### Host-facing module API
Codex should design `Grafitt-Module.h` so that modules interact with the host through a **host API table** rather than ad hoc symbol imports.

Prefer a design where:
- the host passes a `GM_HostAPI` table into `GM_ModStart`
- the module returns or registers one or more capability tables
- each capability table is versioned independently
- optional capabilities can be absent without breaking the whole module

This allows Grafitt to evolve plugin support without repeatedly breaking modules.

Host API areas may include:
- logging
- memory management hooks if needed
- allocator negotiation if needed
- graph registration or graph-type factories
- algorithm registration
- serializer/deserializer registration
- visualization/export registration
- rewrite-rule registration
- e-graph or equality-saturation integration points
- query/matcher registration
- plugin-local configuration access
- diagnostic/event reporting

#### Capability-oriented design
Do not make every plugin look identical internally. Instead, define a **capability-driven module model**.

Possible capabilities include:
- graph algorithm provider
- traversal/pathfinding provider
- decomposition/covering provider
- serializer/deserializer provider
- GBIN extension provider
- JSON/XML/YAML/S-expression codec provider
- DOT/TikZ exporter
- rewrite-rule provider
- e-graph engine extension
- query or matcher provider
- graph isomorphism/equality provider
- instrumentation/telemetry provider

A module should declare capability flags in metadata and then expose matching capability tables during startup.

#### Versioning strategy
Codex should implement an explicit ABI/versioning strategy.

At minimum:
- define a global module ABI version
- define host-side compatibility checks before initialization
- reject incompatible modules cleanly
- surface useful diagnostics for version mismatches

Prefer:
- semantic versioning for module identity
- separate ABI versioning from feature versioning
- per-capability interface versions where useful
- reserved extension points for future growth

Do not rely on informal “it probably matches” loading.

#### Lifecycle design
The plugin lifecycle should be carefully defined and documented.

A good baseline lifecycle is:
1. host discovers candidate module
2. host loads shared library
3. host reads `GM_ModMetadata`
4. host validates ABI version and capability compatibility
5. host resolves required symbols
6. host calls `GM_ModStart`
7. module registers capabilities/services
8. host marks module active
9. host later calls `GM_ModEnd`
10. host unloads module

Codex should define behavior for:
- partial startup failure
- duplicate module names
- duplicate capabilities
- dependency failure
- teardown after partial initialization
- host shutdown ordering
- repeated load/unload if supported
- module refusal to start

#### Dependency model
Support module dependencies in a controlled way.

If dependency support is added, metadata should be able to declare:
- required modules
- optional modules
- required capabilities
- minimum compatible versions

The host should remain responsible for:
- dependency graph validation
- cycle detection
- topological initialization order
- reporting missing or incompatible dependencies

Do not let plugins recursively load arbitrary plugins on their own unless that behavior is explicitly designed and sandboxed.

#### Error handling
The plugin ABI should use explicit status reporting.

Prefer:
- `GM_Status` or equivalent result codes
- structured error payloads where useful
- host-owned diagnostic sinks
- clear separation between fatal load errors and recoverable registration errors

Never allow C++ exceptions to propagate across the module ABI boundary.

If internal C++ code throws, catch it before crossing the boundary and convert it into a stable error result.

#### Memory and ownership
Ownership rules must be explicit.

Codex should define:
- who allocates
- who frees
- which side owns returned strings/buffers
- whether the host provides allocators
- whether module-allocated memory may be released by the host directly

Prefer one of these disciplined models:
- host allocates and module fills
- module allocates and module frees through an exported free function
- all cross-boundary interactions use host-provided allocator callbacks

Do not leave ownership implicit.

#### Threading and reentrancy
The module ABI should document concurrency semantics.

Metadata or capability descriptors should indicate whether a module is:
- single-thread-only
- externally synchronized
- internally thread-safe
- reentrant for concurrent host calls

If Grafitt may run traversal, matching, serialization, or rewrite jobs concurrently, the plugin system should not leave thread-safety ambiguous.

#### Security and trust model
Treat plugins as privileged native code unless a stricter sandbox exists.

Codex should document that modules are trusted by default unless the host later adds isolation.

Still, improve defensive handling:
- validate metadata before use
- reject malformed capability tables
- validate sizes and version fields
- avoid unbounded string assumptions
- isolate module load failures cleanly
- avoid use-after-unload hazards
- avoid dangling function-table references after teardown

If plugin search paths are implemented, document safe search behavior and avoid surprising implicit loading from unsafe directories.

#### Registration domains specific to Grafitt
The plugin system should be designed to extend Grafitt’s major subsystems cleanly.

It should be possible for plugins to register or contribute:
- graph algorithms
- traversal and shortest-path engines
- topological sorting and decomposition/covering algorithms
- graph matching and DSLUtils-backed matching helpers
- rewrite rules and rewrite strategies
- e-graph analyses and extraction strategies
- graph isomorphism/equality engines
- serialization formats and codecs
- GBIN readers/writers
- GraphViz DOT exporters
- TikZ exporters
- query processors or query extensions

Prefer registration by named capability descriptors instead of hard-coded switch statements in the host.

#### Documentation requirements
This subsystem needs extensive documentation.

Add Doxygen-friendly documentation for:
- all ABI structs
- all exported symbols
- all status codes
- lifecycle rules
- ownership rules
- threading guarantees
- versioning policy
- capability registration flows

Also provide at least one minimal example plugin and one more advanced plugin, if the repository structure supports examples.

#### Testing expectations
If the project has tests or examples, add coverage for:
- successful module load
- ABI version mismatch rejection
- missing symbol rejection
- duplicate module handling
- dependency ordering
- partial initialization failure
- capability registration/unregistration
- unload safety
- serialization/export plugin registration
- rewrite/e-graph plugin registration

#### Design preference
Prefer a small, disciplined, long-lived ABI over a large fragile one.

The goal is to make Grafitt extensible in a way that supports serious external modules for:
- algorithms
- serialization
- visualization
- rewriting
- e-graphs
- matching
- analysis

Do not stop at “dynamic loading works.” Build a plugin system that is diagnosable, versioned, capability-oriented, and maintainable.

### 13. C++ plugin framework via `Grafitt-Plugin.hpp`
Add and/or strengthen a **C++-native plugin/pass framework** centered on `Grafitt-Plugin.hpp`.

This is distinct from the C ABI module system in `Grafitt-Module.h`.

- `Grafitt-Module.h` is for **shared-library, runtime-loaded, C99-compatible modules**
- `Grafitt-Plugin.hpp` is for **header-only, C++-based, in-process extensions**
- the C++ framework should feel closer to **LLVM-style passes**, analyses, and extension points

Treat these as complementary systems, not competing ones.

#### Purpose
`Grafitt-Plugin.hpp` should define a collection of **abstract classes, concepts, base interfaces, pass managers, analysis handles, and registration points** that library users can derive from to build extensions directly in C++.

The goal is to support:
- reusable graph passes
- analysis passes
- transformation passes
- rewrite passes
- e-graph/equality-saturation passes
- serialization/export extensions
- validation and diagnostics passes
- optimization pipelines
- decomposition/covering passes
- matching/query passes

This framework should make Grafitt extensible even when users do not want dynamic shared-library plugins.

#### Design goals
Codex should make this framework:
- header-only
- modern C++20
- type-safe
- composable
- low-boilerplate
- well-documented
- suitable for library embedding
- suitable for static linking
- friendly to both one-off custom passes and reusable ecosystems of extensions

Prefer a principled pass architecture over scattered callback hooks.

#### Core architecture
Codex should design `Grafitt-Plugin.hpp` around a pass/extension model with clear roles.

Possible core interface families include:
- `IGraphPass`
- `IAnalysisPass`
- `ITransformPass`
- `IRewritePass`
- `IEGraphPass`
- `IMatchingPass`
- `ISerializationPlugin`
- `IVisualizationPlugin`
- `IValidationPass`
- `IPipelineExtension`

The exact names can follow the library’s style, but the framework should clearly separate:
- analyses that compute facts
- transforms that mutate or rebuild graphs
- rewrites that apply rules
- exporters/serializers that render data
- pipeline extensions that register or orchestrate passes

#### Pass model
The framework should support pass execution over relevant subjects such as:
- imperative graphs
- persistent graphs
- graph views
- query ASTs if applicable
- rewrite IR / rule sets
- e-graphs
- decomposition/covering results

Passes should have clearly defined contracts:
- input type
- output type
- mutation policy
- required analyses
- invalidated analyses
- error-reporting mode
- determinism expectations where relevant

Do not make every pass return `void` if richer results are appropriate.

#### Analysis and invalidation
A serious pass system should support analyses as first-class citizens.

Codex should add facilities for:
- analysis result caching
- invalidation after transforms
- dependency tracking between analyses
- optional lazy recomputation
- explicit preservation sets where useful

If a pass transforms a graph, it should be able to declare what analyses remain valid and what must be recomputed.

LLVM-style inspiration is welcome here: treat analyses as reusable computed facts, not ad hoc helper calls.

#### Pass managers and pipelines
Add pass managers capable of orchestrating execution.

Possible layers include:
- a simple sequential pass runner
- a transformation pipeline builder
- a rewrite pipeline
- an optimization pipeline
- an analysis manager
- specialized managers for e-graphs or decomposition workflows

Prefer APIs that allow:
- composing passes declaratively
- mixing analysis and transform passes
- attaching diagnostics/logging hooks
- configuring strategies and options
- building reusable “standard pipelines”

This should support both:
- simple one-pass usage
- sophisticated multi-stage optimization or normalization pipelines

#### Abstract base classes and concepts
Since you want abstract-class-based extensions, Codex should provide robust abstract interfaces.

These should include:
- virtual destructors
- clearly documented lifecycle and ownership rules
- const-correct APIs for analysis-style interfaces
- mutation-aware APIs for transform-style interfaces
- result/status types where needed

At the same time, Codex may add C++20 concepts or adapter helpers so users can write lower-boilerplate pass objects and have them wrapped into the abstract interface system.

In other words:
- abstract classes are the stable authoring model
- concepts/adapters may provide ergonomic sugar

#### Registration and discovery
Because this framework is header-only and in-process, “plugin registration” is different from dynamic module loading.

Prefer mechanisms such as:
- explicit pass registration objects
- registry containers
- compile-time registries where appropriate
- named pass factories
- pipeline builder hooks
- extension-point callbacks

Users should be able to:
- derive a pass
- register it with a registry or manager
- invoke it by type, by name, or via a pipeline builder
- bundle groups of related passes together

Do not rely entirely on fragile global static initialization unless it is optional and clearly controlled.

#### Extension points specific to Grafitt
The framework should be able to represent extensions for all major Grafitt domains, including:
- traversal and pathfinding
- topological sorting
- graph covering/decomposition
- graph matching
- DSLUtils-backed matching integration
- graph isomorphism/equality
- serialization/deserialization
- GBIN support
- JSON/XML/YAML/S-expression codecs
- GraphViz DOT export
- TikZ export
- rewrite strategies
- rewrite-rule scheduling
- e-graph analyses
- equality saturation strategies
- extraction/cost models
- validation, linting, and canonicalization

This should feel like a coherent ecosystem, not a bag of unrelated interfaces.

#### Rewrite and e-graph integration
Because Grafitt has rewrite ambitions, the pass framework should integrate tightly with rewriting and equality saturation.

Codex should support pass types such as:
- rule canonicalization passes
- rewrite application passes
- strategy-selection passes
- saturation passes
- e-class analysis passes
- extractor passes
- cost-model passes
- proof/explanation generation passes if the architecture later supports them

Passes should be composable so users can build high-level workflows like:
- parse/query
- normalize
- rewrite
- saturate
- extract
- validate
- serialize/export

#### Analysis result types
Encourage strong result modeling.

Instead of raw primitive returns everywhere, prefer named result types for:
- shortest-path summaries
- decomposition metadata
- match results
- isomorphism witnesses
- cycle extraction
- rewrite statistics
- saturation summaries
- serialization diagnostics

This makes passes easier to compose and reason about.

#### Error handling
The C++ plugin framework may use normal C++ library error handling internally, but it still needs disciplined contracts.

Prefer:
- domain-specific exceptions only where the surrounding library already uses them
- status/result objects where workflow composition benefits from them
- explicit documentation of which passes may throw
- clean diagnostics hooks for non-fatal issues

Do not hide important pass failures behind silent no-ops.

#### Threading and execution model
Document whether passes are:
- single-threaded
- reentrant
- safe for concurrent execution on distinct graphs
- safe for concurrent execution within one pass manager

If parallel execution is introduced later, the framework should already have clean enough interfaces to support it.

#### Pass metadata
Consider giving passes lightweight metadata such as:
- pass name
- category
- version
- stability level
- required analyses
- preserved analyses
- supported graph kinds
- capability tags

This improves diagnostics, tooling, and pipeline introspection.

#### Tooling and diagnostics
A production-ready pass framework should support introspection.

Add facilities for:
- listing registered passes
- printing pipeline structure
- tracing pass execution
- collecting timing/statistics
- collecting invalidation information
- debugging failed rewrites/matches/decompositions

These facilities should be optional but easy to enable.

#### Interop with `Grafitt-Module.h`
The C++ plugin framework and the C ABI module system should interoperate where sensible.

For example:
- a shared-library module may register C++ pass factories into the host
- a C++ pass may be wrapped as a module-exposed capability
- registries should be designed so module-loaded extensions and statically linked extensions can coexist

But do not tightly couple the two systems so much that either one becomes hard to use independently.

#### Documentation requirements
Add extensive Doxygen-friendly documentation to `Grafitt-Plugin.hpp`.

Document:
- every abstract base class
- pass categories
- lifecycle and ownership rules
- registration model
- analysis invalidation rules
- pipeline composition model
- example pass implementations
- extension-point contracts

If possible, include at least:
- one minimal analysis pass example
- one transform/rewrite pass example
- one pipeline example

#### Design preference
Aim for something that feels like:
- LLVM-style passes in spirit
- modern C++20 in implementation
- lightweight enough for a header-only library
- flexible enough to support Grafitt’s algorithms, rewriting, e-graphs, matching, serialization, and visualization systems

Do not reduce `Grafitt-Plugin.hpp` to a handful of abstract classes with no orchestration model. Build a real extension framework around them.


## Documentation Requirements

Add **a lot of docstrings** in a Doxygen-friendly style.

All substantial public APIs should have documentation covering:
- purpose
- template parameter meaning
- preconditions
- postconditions
- complexity
- exception behavior
- directed/undirected semantics
- weight semantics where relevant
- examples where useful

Prefer documenting public types and free functions as they are introduced, not in a cleanup pass much later.

If existing public APIs are undocumented and you are already modifying them, add documentation there too.

## Quality Bar

Any nontrivial algorithm addition should include:
- complexity notes
- edge-case handling
- path/cycle reconstruction where relevant
- tests/examples if the repo has a place for them
- no hidden assumptions about contiguous integer vertex IDs unless the library already requires that

Avoid:
- baking algorithms around one concrete vertex type
- using global state
- adding non-header-only build requirements
- introducing fragile macros
- over-coupling algorithms to one graph representation when they can work over the existing graph interface

## Preferred Implementation Style

- Prefer small internal helpers over giant monolithic functions.
- Prefer clear data-flow and reconstructible results.
- Prefer named result structs for richer algorithms, for example:
  - distance maps
  - predecessor maps
  - extracted cycle/path
  - status enums
- When an algorithm can fail for a principled reason, return a rich result or throw an existing domain-specific exception if that matches current library conventions.

## Serialization Guidance

Before touching serialization:
1. inspect existing `gbin` facilities
2. inspect current SerdeTk usage
3. identify supported archive/format abstractions
4. extend them consistently

Do not guess the SerdeTk API.

If JSON/XML/YAML/S-expression support already exists:
- improve coverage
- improve docs
- improve round-trip testing
- improve schema clarity
- improve error reporting

If GBIN exists but is thin:
- strengthen it
- make it more robust
- ensure deserialization has strong validation paths

## Rewrite + E-graph Guidance

The rewrite system and e-graph system should cooperate.

Codex should try to make it possible to:
- define rewrite rules cleanly
- apply them directly
- load them into equality saturation workflows
- extract optimized or normalized results
- feed results back into graph structures or term-like representations

If the current rewrite engine already has matcher infrastructure, reuse it.

## GSS (Graph-Structured Stack)

You are to add a GSS, or Graph-Structured Stack, with all its operations.

## Performance Guidance

Be mindful of performance, but do not sacrifice correctness or clarity prematurely.

Prefer:
- reasonable asymptotic choices
- avoiding needless graph copies in imperative workflows
- preserving persistent semantics cleanly in persistent workflows
- avoiding quadratic behavior where linearithmic or near-linear solutions are standard

Document when an algorithm is intentionally conservative or approximate.

## Refactoring Guidance

Refactor when needed, but keep it disciplined.

Good refactors:
- extracting reusable helpers
- reducing duplication between algorithms
- improving namespace organization
- clarifying serialization boundaries
- making rewrite/e-graph APIs composable

Bad refactors:
- renaming major APIs gratuitously
- replacing established public semantics without migration
- introducing a whole new architecture without necessity

## Deliverables Codex Should Prefer

When implementing major features, Codex should ideally leave behind:
- the feature implementation
- Doxygen-ready public documentation
- small usage examples or tests if the repository already supports them
- careful error messages
- consistent naming and formatting

## Final Instruction

Treat Grafitt as a library that should evolve from a simple graph toolkit into a **production-ready, documented, extensible, header-only graph platform**.

Do not merely bolt on algorithms. Improve the coherence of:
- traversal
- shortest paths
- sorting/order algorithms
- serialization
- visualization
- rewrite infrastructure
- e-graphs
- equality saturation
- graph equality/isomorphism
- graph covering

When in doubt, inspect the existing code first, extend what is already there, and leave the public API cleaner and stronger than you found it.


# Token Economy Rules

The agent must optimize for:
- minimal token consumption;
- maximal information density;
- low conversational overhead;
- academic precision;
- implementation usefulness.

The agent must behave like:
- a systems engineer;
- a compiler engineer;
- a technical reviewer;
- an RFC author.

The agent must NOT behave like:
- a tutor;
- a marketer;
- a motivational speaker;
- a conversational assistant.

---

# Core Principles

## 1. Prefer Dense Technical Writing

BAD:

"The reason this happens is because the compiler internally needs to understand the vector lanes before lowering."

GOOD:

"Lowering requires lane-width canonicalization."

---

## 2. No Conversational Padding

Forbidden:
- "Great question"
- "Excellent point"
- "Absolutely"
- "Sure"
- "Of course"
- "You're right"
- "Let's explore"
- "Here's the thing"

Responses must begin immediately with technical content.

---

## 3. No Redundant Restatement

Do not restate:
- the prompt;
- previous answers;
- obvious implications.

BAD:

"Since you are building a vector extension system..."

GOOD:

"Use semantic vector operations."

---

## 4. Prefer Lists Over Paragraphs

Prefer:

```text
- legalization;
- lowering;
- canonicalization;
````

instead of prose.

---

## 5. Avoid Tutorial Tone

Do not teach incrementally unless explicitly requested.

Assume:

* compiler literacy;
* systems programming literacy;
* IR familiarity;
* architecture familiarity.

---

## 6. Compress Explanations

BAD:

"Predication is important because some architectures like AVX512 use masks for execution."

GOOD:

"Predication models masked execution semantics."

---

## 7. Prefer Terminology Over Explanation

Use precise terms directly:

* legalization;
* SSA;
* dominance;
* lane packing;
* vector splitting;
* predication;
* swizzle;
* canonicalization.

Avoid defining common terms unless asked.

---

# Response Structure

Preferred order:

1. Architecture;
2. Constraints;
3. Tradeoffs;
4. Recommended implementation;
5. Failure modes.

Avoid:

* introductions;
* summaries;
* conclusions.

---

# Code Rules

## 1. Prefer Minimal Examples

BAD:

```c
int add(int a, int b) {
    return a + b;
}
```

GOOD:

```c
vadd <8xi32>
```

---

## 2. Omit Boilerplate

Avoid:

* includes;
* guards;
* trivial constructors;
* repetitive wrappers.

Unless specifically requested.

---

## 3. Prefer Semantic Examples

GOOD:

```text
ReduceAdd
Shuffle
Gather
```

BAD:

```text
VPADDD
VPSHUFD
```

unless discussing backend lowering.

---

# Architecture Rules

## 1. Prefer Semantic IR

Always distinguish:

* semantic operations;
* machine instructions.

---

## 2. Prefer Declarative Systems

Favor:

* tables;
* schemas;
* YAML;
* metadata-driven lowering.

Avoid:

* hardcoded switch forests;
* backend duplication.

---

## 3. Separate Layers Aggressively

Keep separate:

* semantics;
* legality;
* lowering;
* register layout;
* instruction encoding;
* optimization.

---

# Token Suppression Rules

The agent must suppress:

* praise;
* hedging;
* rhetorical questions;
* motivational phrasing;
* conversational transitions.

Forbidden:

* "I think"
* "Probably"
* "Maybe"
* "It might"
* "In my opinion"

Use direct assertions.

---

# Brevity Rules

If a concept can be expressed in:

* 1 sentence instead of 4;
* 1 list instead of prose;
* 1 term instead of explanation;

the shorter form is mandatory.

---

# Academic Style Rules

Prefer:

* RFC style;
* compiler documentation style;
* ISA manual style;
* research-paper density.

Avoid:

* blog style;
* tutorial style;
* social tone;
* conversational framing.

---

# Refactoring Rules

When reviewing architecture:

Prefer:

* decomposition;
* canonical forms;
* normalization;
* declarative metadata;
* semantic abstraction.

Reject:

* stateful implicit behavior;
* hidden lowering;
* machine-specific semantics in IR;
* duplicated legality logic.

---

# Optimization Rules

Always prioritize:

1. canonicalization;
2. legality;
3. lowering quality;
4. data layout;
5. register pressure;
6. instruction selection.

Do not over-focus on:

* syntax;
* naming;
* micro-abstractions.

---

# Communication Rules

Default answer length:

* short.

Increase detail only if:

* explicitly requested;
* architectural complexity demands it;
* ambiguity exists.

One precise paragraph is preferred over five mediocre paragraphs.

---

# Failure Modes To Avoid

* tutorial verbosity;
* repeating the prompt;
* excessive examples;
* excessive prose;
* anthropomorphic explanations;
* motivational wording;
* unnecessary historical context;
* excessive caveats.

The agent must optimize for:

* density;
* precision;
* architecture;
* implementation value;
* token economy.

