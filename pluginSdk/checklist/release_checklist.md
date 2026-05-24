# Plugin Release Checklist

- `plugin.toml` uses manifest schema `1`.
- Public roots are declared under `[stdlib]`.
- Compiler hooks declare fallback and source-map policy.
- Runtime kernels declare ABI, typed failure, and no-coredump policy.
- Lockfile fingerprint, checksum, and signature are reproducible.
- Positive executable fixture passes.
- Negative fixtures cover missing capability, stale lockfile, and bad ABI.
- Performance proof compares plugin fast path against fallback.
- Documentation and examples stay outside compiler/runtime product code.

