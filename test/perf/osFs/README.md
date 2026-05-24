# OS/FS Perf Proof

Measured OS/FS benchmarks live here, not in product runtime/compiler code.

The perf suite must cover:

- path join/normalize allocation behavior
- file streaming throughput and bounded memory use
- atomic write and rename paths
- process spawn/wait/output capture ceilings
- terminal detection hot paths
- signal wait/wake latency
- capability denial overhead

