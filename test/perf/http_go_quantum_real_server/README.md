# HTTP Go vs QuantumLang Real Server Benchmark

This directory is for the real socket benchmark between a Go HTTP server and a
QuantumLang HTTP server.

It is intentionally separate from the older proof fixtures:

- `plugins/httpAccelerated/test/perf/goComparisonBenchmark.qn` is a comparison
  shape fixture.
- `plugins/httpAccelerated/test/perf/native/safeRealServerBenchmark.cpp` is a
  C++ native safe server benchmark.

This benchmark uses two end-user projects with the same route and response:

```text
GET /users
{"users":[{"id":1,"name":"salam"}]}
```

## Layout

```text
go_server/
  go.mod
  main.go

quantum_server/
  quantum.toml
  src/main.qn

bench/
  go.mod
  main.go

scripts/
  run_safe.sh

reports/
  latest.txt
```

## Run

Safe local run:

```bash
bash qtlc/qtlang/test/perf/http_go_quantum_real_server/scripts/run_safe.sh
```

The default caps are intentionally small:

```text
requests=256
concurrency=8
go port=18080
quantum port=18081
```

Override caps:

```bash
QTLC_HTTP_BENCH_REQUESTS=1000 \
QTLC_HTTP_BENCH_CONCURRENCY=16 \
bash qtlc/qtlang/test/perf/http_go_quantum_real_server/scripts/run_safe.sh
```

## Claim Rule

The benchmark only claims QuantumLang is faster when both real servers run and
the measured QuantumLang requests/second is at least 10% higher than Go.

If the QuantumLang package builds but does not listen on a socket yet, the report
must say:

```text
quantumStatus=blocked
claim=blocked
```

No product speed claim is allowed from a fixture or from a server that did not
actually accept requests.
