# grafitt

`grafitt` is a header-first C++20 graph toolkit inspired by OCamlGraph.

Current repository focus:
- explicit mutable (`imperative_graph`) and immutable (`persistent_graph`) APIs
- small builder helpers
- traversal/query utilities
- Queryfitt AST + minimal textual parser boundary
- rewrite and GBIN hooks prepared for incremental implementation

## Status

This repository is in an incremental build-out phase.

Implemented now:
- core graph storage and operations
- mutable and immutable graph interfaces
- OCamlGraph-style iteration/fold helpers
- core algorithms (`reachable`, `shortest_path`, `bfs_order`)
- Queryfitt AST and one minimal Matcheroni-backed parser path (`path a -> b`)
- rewrite rule placeholder API
- schema lookup hooks for GBIN (`GRAFITT_SCHEMA_DIR`, fallback `specs/`)

Intentionally minimal/placeholders for later slices:
- full textual Queryfitt grammar execution
- concrete rewrite execution engine
- concrete GBIN byte format serializer/deserializer

## Layout

- `Grafitt.hpp`: main single-header library
- `specs/Queryfitt.ebnf`: textual Queryfitt grammar skeleton
- `specs/PatternMatching.schema.json`: rewrite pattern schema
- `specs/GBIN.sktl`: GBIN schema descriptor
- `manual/`: concise implementation-aligned manual chapters
- `examples/`: usage snippets and query text files

## Requirements

- C++20 compiler
- optional local headers (auto-detected):
  - `DSLUtils.hpp`
  - `SerdeTk.hpp`
  - `matcheroni/Matcheroni.hpp` + `matcheroni/Utilities.hpp`

The header compiles without optional integrations; corresponding features degrade to narrow stubs.

## Quick Start

```cpp
#include "Grafitt.hpp"

int main() {
    grafitt::imperative_graph<int> g;
    g.add_vertex(1);
    g.add_edge(1, 2);
    return g.mem_edge(1, 2) ? 0 : 1;
}
```

Build:

```bash
g++ -std=c++20 -I. your_file.cpp -o your_app
```

## Graph APIs

### Mutable graph (`imperative_graph`)

Use when in-place updates are desired.

```cpp
#include "Grafitt.hpp"
#include <string>

int main() {
    grafitt::imperative_graph<std::string> g;
    g.add_vertex("a");
    g.add_edge("a", "b");
    g.add_edge("b", "c");

    g.remove_edge("b", "c");

    bool has_ab = g.mem_edge("a", "b");
    bool has_bc = g.mem_edge("b", "c");
    return (has_ab && !has_bc) ? 0 : 1;
}
```

### Immutable graph (`persistent_graph`)

Use when updates must return new graph values without mutating earlier versions.

```cpp
#include "Grafitt.hpp"

int main() {
    grafitt::persistent_graph<int> g0;
    auto g1 = g0.add_vertex(10);
    auto g2 = g1.add_edge(10, 20);

    // g0 unchanged, g1 has only vertex 10, g2 has edge 10->20
    return (!g0.mem_vertex(10) && g1.mem_vertex(10) && g2.mem_edge(10, 20)) ? 0 : 1;
}
```

### Builders

```cpp
#include "Grafitt.hpp"

int main() {
    using G = grafitt::imperative_graph<int>;
    auto g = grafitt::builder::imperative_builder<G>()
        .vertex(1)
        .vertex(2)
        .edge(1, 2)
        .build();

    return g.nb_edges() == 1 ? 0 : 1;
}
```

## Algorithms

Available helpers in `grafitt::algo` include:
- `reachable(g, src, dst)`
- `shortest_path(g, src, dst)`
- `bfs_order(g, root)`
- `degree(g, v)`
- `vertices(g)`, `edges(g)`

Example:

```cpp
#include "Grafitt.hpp"

int main() {
    grafitt::imperative_graph<int> g;
    g.add_edge(1, 2);
    g.add_edge(2, 3);

    auto ok = grafitt::algo::reachable(g, 1, 3);
    auto path = grafitt::algo::shortest_path(g, 1, 3);
    return (ok && path && path->size() == 3) ? 0 : 1;
}
```

## Queryfitt (Current Slice)

### C++ query construction

```cpp
#include "Grafitt.hpp"

int main() {
    auto q = grafitt::queryfitt::shortest_path_between("alice", "bob");
    (void)q;
    return 0;
}
```

### Text query parsing

`parse_text()` currently supports a narrow Matcheroni-backed structural form:
- valid shape: `path <ident> -> <ident>`

```cpp
#include "Grafitt.hpp"

int main() {
    auto ok = grafitt::queryfitt::parse_text("path a -> b");
    auto bad = grafitt::queryfitt::parse_text("traverse from a");
    return (ok.has_value() && !bad.has_value()) ? 0 : 1;
}
```

This keeps parsing/semantics separation explicit:
- Matcheroni checks structure
- AST conversion happens after a successful match

## Rewriting (Placeholder API)

```cpp
#include "Grafitt.hpp"

int main() {
    grafitt::rewrite::rule r {
        .name = "noop",
        .match = grafitt::queryfitt::shortest_path_between("a", "b"),
        .replacement = "no-op"
    };

    grafitt::imperative_graph<int> g;
    auto g2 = grafitt::rewrite::apply_once(g, r); // currently returns input graph
    (void)g2;
    return 0;
}
```

## GBIN Hooks and Schema Lookup

`grafitt::gbin` provides narrow integration points:
- `schema_root()`
- `default_schema_path()`
- `serialize(graph)` (placeholder)
- `deserialize<Graph>(bytes)` (placeholder)

Environment variable:
- `GRAFITT_SCHEMA_DIR`: custom schema directory
- fallback: `specs/`

Example:

```cpp
#include "Grafitt.hpp"
#include <iostream>

int main() {
    std::cout << grafitt::gbin::default_schema_path().string() << "\n";
    return 0;
}
```

## Specs and Manual

- Query grammar: `specs/Queryfitt.ebnf`
- Pattern schema: `specs/PatternMatching.schema.json`
- GBIN schema descriptor: `specs/GBIN.sktl`
- manual index: `manual/README.md`

## Smoke Test Ideas

After editing `Grafitt.hpp` or parser-related code, run at least:
- header include/compile smoke
- one mutable + immutable behavior check
- one valid + invalid Queryfitt parse check

A minimal compile smoke:

```bash
cat >/tmp/grafitt_smoke.cpp <<'CPP'
#include "Grafitt.hpp"
int main() { return 0; }
CPP

g++ -std=c++20 -I. /tmp/grafitt_smoke.cpp -o /tmp/grafitt_smoke
```

## Notes on Naming

Repository convention currently uses `Grafitt.hpp` (capital `G`) as the main include path.
