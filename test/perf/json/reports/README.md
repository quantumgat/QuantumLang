# JSON Benchmark Reports

Status: scaffold

Measured reports must include:

```text
target triple
CPU model
SIMD features enabled
input corpus hash
input size
warmup count
iteration count
allocation count
bytes per second
cycles per byte when available
malformed corpus error rate
compiler flags
baseline version or source hash
```

No JSON speed claim is complete until a report in this directory satisfies the
required evidence rows.
