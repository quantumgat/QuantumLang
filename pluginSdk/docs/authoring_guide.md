# Plugin Authoring Guide

## Directory Shape

```text
my-plugin/
  plugin.toml
  stdlib/
  compiler/
  runtime/
  test/
```

Only `plugin.toml` and `stdlib/` are required for a simple public API plugin.
Compiler and runtime sections are added only when the plugin needs lowering,
target specialization, runtime kernels, or typed failure handling.

## Manifest Ownership

`[stdlib]` declares the public source root that the user imports.

`[compiler]` declares compile-time behavior: types, facts, hooks, lowering
passes, diagnostics, source maps, and fallback policy.

`[runtime]` declares execution behavior: kernels, ABI level, lifecycle,
target gates, typed failures, and no-coredump requirements.

`[proof]` declares the release gates that make performance and safety claims
auditable.

## Safety Contract

Plugins must fail through typed diagnostics. A plugin may be fast, but it must
not make the compiler or runtime crash on missing capability, stale lockfile,
bad target, bad ABI, or runtime kernel failure.

