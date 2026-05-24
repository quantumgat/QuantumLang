# JSON Proof Fixtures

Status: scaffold

This directory owns non-benchmark proof for:

```text
JsonCodec derive path
typed codec lowering
field matcher generation
unknown/null/missing field policy
zero-copy lifetime safety
path-aware diagnostics
no public Qn runtime binding leak
```

Current proof rows:

```text
conformanceFixtureProof.qn
negativeFixtureProof.qn
exactNumberFixtureProof.qn
scannerFallbackFixtureProof.qn
zeroCopyViewFixtureProof.qn
```

Measured benchmark proof still belongs under `qtlc/qtlang/test/perf/json`.
