# JSON Performance Fixtures

Status: scaffold

This directory owns measured JSON benchmark proof for:

```text
typed decode
typed encode
zero-copy view decode
dynamic JsonValue parse
malformed input diagnostics
SIMD/scalar fallback comparison
simdjson-style baseline comparison
```

No product speed claim is complete until this directory contains measured
fixtures and reports.

Current proof rows:

```text
benchmarkCase.qn
baselineComparison.qn
reportEvidence.qn
measuredBenchmarkProof.qn
```

`JsonMeasuredBenchmarkProof.required()` intentionally starts with all measured
flags false. A speed claim becomes valid only after real benchmark reports set
the required evidence fields.
