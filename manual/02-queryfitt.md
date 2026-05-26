# Queryfitt

- `queryfitt::query` models pattern, traversal, path, reachability, and aggregation clauses.
- `queryfitt::parse_text()` now parses the textual form used by `examples/*.qfitt`, including optional `---` metadata blocks and `in "..." { ... }` bodies.
- `queryfitt::execute()` evaluates parsed/native queries:
  - traversal: BFS order (depth-limited)
  - path: shortest path
  - reachable: boolean reachability (bounded when `max_depth > 0`)
  - aggregate:
    - `count vertices`, `count edges`
    - `degree <vertex-label>`
    - `sum degree`, `avg degree`, `min degree`, `max degree`
    - optional `where` filter (simple text predicates: exact match, `== x`, `contains x`)
    - optional `by all|vertex_type`
    - `by vertex_type` returns per-group scalar values keyed by inferred vertex type from vertex labels (`Type:Name` or `:Type`)
  - match: edge-oriented match emission with caller predicates
