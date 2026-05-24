# QuantumLang Plugin SDK Templates

This directory contains SDK examples for third-party plugin authors. Generated
plugins created by `qtlc plugin create` must contain real `.qn` files and a real
`plugin.toml`; users should not see `.template` suffixes in created plugins.

## Templates

- `stdlib`: public QuantumLang API only.
- `compiler`: compiler hooks, type facts, lowering, and diagnostics.
- `runtime`: runtime kernels plus a small stdlib facade.
- `capability`: public API plus compiler hooks, runtime kernels, tests, proof,
  and performance gates.
- `framework`: high-level framework package with optional compiler/runtime hooks.

## Developer Docs

- `docs/developer_workflow.md`: end-to-end author workflow.
- `docs/flow/plugin_load_flow.md`: package, manifest, compiler, runtime, and
  proof load flow.
- `examples/README.md`: manual and planned CLI flows.
- `checklist/release_checklist.md`: release gate checklist.

## Rules

- Keep public API under `stdlib/`.
- Put compiler behavior under `compiler/`.
- Put runtime behavior under `runtime/`.
- Put proof, negative tests, and benchmarks under `test/`.
- Keep plugin manifests explicit about safety, source maps, fallback, and
  no-coredump behavior.
- Prefer `qtlc plugin create <name> --kind <kind> [--dir <parent>]` for real
  plugin scaffolds.
