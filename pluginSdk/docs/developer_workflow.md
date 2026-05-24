# Plugin Developer Workflow

## Start

Choose the smallest kind that matches the plugin.

```text
stdlib       public API only
compiler     compiler hooks, type facts, lowering, and diagnostics
runtime      runtime kernel plus small public facade
capability   public API plus compiler hooks and runtime kernels
framework    high-level framework package with optional hooks
```

Create the plugin with the CLI:

```text
qtlc plugin create myplugin --kind capability
qtlc plugin create myplugin --kind framework --dir packages
```

The generated plugin contains real `.qn` files and `plugin.toml`; no user-facing
`.qn.template` files are emitted. `qtlc plugin new` remains a compatibility
alias for old scripts.

## Build The Public API

Put user-facing QuantumLang code under `stdlib/`.

```text
stdlib/
  pluginTemplate/
    mod.qn
```

`mod.qn` owns user-facing exports. `plugin.toml` declares public roots so the
compiler can detect namespace conflicts before loading the plugin.

## Add Compiler Behavior

Use `compiler/` only when the plugin needs compile-time behavior:

- new type facts
- intrinsic lowering
- target specialization
- diagnostics
- source maps
- fallback selection

The compiler core loads the plugin through generic hooks. It must not contain a
package-name-specific path for a plugin.

## Add Runtime Behavior

Use `runtime/` only when the plugin needs execution kernels:

- target-gated kernels
- dynamic library loading
- typed failure conversion
- lifecycle open/close
- no-coredump failure handling

Runtime kernels must return typed failure. They must not crash the host runtime.

## Add Proof

Use `test/` for proof:

- positive executable fixture
- negative diagnostics
- stale lockfile check
- missing capability check
- fallback check
- performance benchmark

Performance claims belong in tests and proof reports, not product source.

## Current CLI Status

The current source has loader, package graph, sandbox, runtime kernel, fixture,
SDK-template, and package-owned CLI contracts. The release driver accepts
`qtlc plugin new`, `qtlc plugin check`, `qtlc plugin test`,
`qtlc plugin package`, and `qtlc plugin publish` through the generic
package/plugin driver path. Domain-specific compiler/runtime behavior still
belongs to plugin manifests and generic hooks, not hardcoded driver branches.
