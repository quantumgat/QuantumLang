# Plugin Load Flow

```text
plugin.toml
  -> package resolver
  -> manifest parser
  -> lockfile and proof validation
  -> stdlib public-root mount
  -> compiler sandbox hook load
  -> runtime dynamic kernel load
  -> executable and negative fixture gates
```

## Boundaries

- Package resolver finds the plugin and records exact lockfile proof.
- Manifest parser normalizes `[stdlib]`, `[compiler]`, `[runtime]`,
  `[exports]`, and `[proof]`.
- Stdlib mount exposes public roots but does not own compiler/runtime behavior.
- Compiler hooks operate through sandboxed generic extension points.
- Runtime kernels operate through ABI-checked dispatch tables.
- Proof gates stay test-owned.

## Failure Policy

Every load-stage failure must be typed:

- bad manifest
- legacy schema
- stale lockfile
- namespace conflict
- unsupported target
- missing capability
- bad runtime ABI
- kernel load failure

No load-stage failure should become a coredump.

