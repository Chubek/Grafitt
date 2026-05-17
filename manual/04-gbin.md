# GBIN and Schema Lookup

- Schema root lookup checks `GRAFITT_SCHEMA_DIR` first.
- Fallback schema root is `specs/`.
- `gbin::serialize()` and `gbin::deserialize()` are narrow placeholders for later SerdeTk integration.
