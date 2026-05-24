# HTTP Accelerated Plugin Source Layout

Command: `POST-Q134-SELFHOST-HTTP-ACCELERATED-PLUGIN-SOURCE-LAYOUT-001`

Status: complete

This plugin is the official free HTTP accelerator. It does not own the public
HTTP framework API. User code continues to import and use `packages/http`.

## Boundary

```text
packages/http:
  public API
  endpoint, route, extractor, capability, handler, source-map facts

plugins/httpAccelerated:
  consumes package-owned facts
  registers generic compiler hooks
  loads ABI-checked runtime kernels

compiler/runtime core:
  generic plugin hooks only
  no HTTP-specific product or runtime directories
```

Forbidden core paths remain absent:

```text
qtlc/qtlang/compiler/src/product/httpModel/
qtlc/qtlang/compiler/src/backend/native/emit/http/
qtlc/qtlang/runtime/src/http/
```

## Layout

```text
qtlc/qtlang/plugins/httpAccelerated/
  plugin.toml
  stdlib/http/accelerated/
  compiler/facts/
  compiler/hooks/
  compiler/lowering/
  compiler/target/
  compiler/diagnostics/
  compiler/sourcemap/
  runtime/abi/
  runtime/kernels/
  runtime/lifecycle/
  runtime/target/
  runtime/diagnostics/
  test/proof/
  test/perf/
  test/negative/
  docs/
```

## Hot Path Scope

The source layout now includes the first plugin-owned hot-path pipeline:

```text
package facts
  -> route trie lowering
  -> flattened extractor/capability lowering
  -> direct sync handler or async state-machine hook
  -> scatter/gather writer and sendfile handoff
  -> runtime kernel binding
```

Measured performance proof is test-owned under `test/perf`. Product speed
claims remain blocked until release policy accepts the report rows.
