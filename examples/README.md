# Examples

Quick map of the sample programs and query files.

| File | Focus |
|---|---|
| `mutable-graph.cpp` | Mutable graph basics (insert, iterate, count) |
| `builder.cpp` | Builder-based graph construction |
| `write-graph.cpp` | Serialize a small graph to GBIN |
| `write-gbin.cpp` | GBIN serialization helper usage |
| `reachability.cpp` | Reachability and shortest-path algorithms |
| `queryfitt-exec.cpp` | Native Queryfitt query execution |
| `find-pattern.cpp` | Embedded pattern DSL construction |
| `graph-rewriting.cpp` | Rewrite rule application (`apply_once`) |
| `aggregation-count.cpp` | `count vertices` scalar aggregation |
| `aggregation-where.cpp` | Aggregation with `where` filtering |
| `aggregation-by-vertex-type.cpp` | Grouped `count vertices by vertex_type` |
| `aggregation-degree-stats.cpp` | Scalar degree stats (`sum degree`) |
| `aggregation-degree-by-vertex-type.cpp` | Grouped degree stats (`avg degree by vertex_type`) |
| `aggregation-parse-text.cpp` | Text query parse + execute for grouped aggregation |
| `aggregation-kitchen-sink.cpp` | Multiple aggregate forms in one runnable file |
| `aggregation.qfitt` | Minimal textual aggregate query |
| `aggregation-by-type.qfitt` | Textual grouped count query |
| `aggregation-where-by-type.qfitt` | Textual grouped query with `where` |
| `aggregation-degree-by-type.qfitt` | Textual grouped degree query |
| `path.qfitt` | Textual path query |
| `reachability.qfitt` | Textual reachability query |
| `traverse.qfitt` | Textual traversal query |
| `third-wheelers.qfitt` | Textual match-pattern sample |
