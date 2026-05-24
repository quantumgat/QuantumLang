# JSON Conformance Corpus

Status: scaffold

This directory owns JSON conformance cases for:

```text
valid dynamic parse
valid dynamic encode
typed JsonCodec decode/encode
field policy behavior
exact number round trip
path-aware diagnostics
```

The corpus must stay test-owned. Product `std::json`, compiler, and runtime
code must not include benchmark or fixture rows.
