# JSON Negative Fixtures

Status: scaffold

This directory owns JSON negative cases for:

```text
malformed JSON
invalid UTF-8
invalid escapes
depth limit overflow
missing required fields
duplicate fields
unknown field policy
precision loss
non-finite floats
unsupported resource and quantum handles
dangling zero-copy views
```

Every negative fixture must fail with a typed diagnostic and no coredump.
