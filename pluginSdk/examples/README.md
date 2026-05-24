# Plugin SDK Examples

Examples in this directory describe expected developer commands and repository
shape. They are documentation examples, not executable source.

## Manual Flow

```text
copy qtlc/qtlang/pluginSdk/templates/capability_pack my-plugin
rename *.template files
edit plugin.toml
write stdlib/pluginTemplate/mod.qn
write compiler hooks only if needed
write runtime kernels only if needed
add positive, negative, fallback, and perf proof
```

## Planned CLI Flow

```text
qtlc plugin new my-plugin --template capability-pack
qtlc plugin check
qtlc plugin test
qtlc plugin package
qtlc plugin publish
```

These commands are a planned tooling layer. The template contracts exist first
so the implementation has a stable shape.

