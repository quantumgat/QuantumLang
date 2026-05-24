# Core Net Proof Fixtures

This directory owns networking proof fixtures. Product compiler/runtime source
must expose contracts and diagnostics only; benchmark and proof evidence stays
under `test/`.

Required proof families:

- public API policy rejects `std::net::async` and keeps HTTP deferred
- socket handles reject stale and double-close paths without coredumps
- async IO lowering proves no borrowed buffer escapes across await
- TCP/UDP hot paths prove bounded copies and cancellation cleanup
- DNS/TLS surfaces prove timeout, verification, and unsafe-disable diagnostics

