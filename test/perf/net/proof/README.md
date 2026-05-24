# Core Net Perf Proof

Measured networking benchmarks live here, not in product runtime/compiler code.

The perf suite must compare QuantumLang generated networking code against
equivalent C/C++/Go baselines for:

- TCP accept/read/write wakeups
- UDP batch send/receive
- zero-copy buffer views
- per-core reactor sharding
- cancellation cleanup
- backpressure events
- no-coredump failure paths

