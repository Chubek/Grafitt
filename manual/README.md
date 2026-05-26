# Grafitt Manual

This manual tracks implemented behavior in the current header.

## Chapters

- `01-graphs.md`: mutable and immutable graph APIs
- `02-queryfitt.md`: query AST and textual parser boundary
- `03-rewrite.md`: rewrite rule shape and concrete `apply_once()` behavior
- `04-gbin.md`: schema location and serialization hooks

## Example Set

- Core usage examples live in `examples/*.cpp`.
- Query text samples live in `examples/*.qfitt`.
- Aggregation-specific samples include grouped `by vertex_type` usage.
