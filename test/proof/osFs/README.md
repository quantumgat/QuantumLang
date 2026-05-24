# OS/FS Proof Fixtures

This directory owns self-hosted OS, filesystem, path, process, terminal,
signal, and platform safety proof fixtures.

Required proof families:

- filesystem streaming and partial-write safe failure
- path normalization without traversal surprises
- process output limits, pipe lifecycle, wait, kill, and signal cleanup
- terminal raw-mode/echo restoration diagnostics
- signal handler registration and wake integration
- `std::sys` raw handle capability denial
- no-coredump diagnostics for every safe API failure path

