# Validation for 0.2.0

Validated on 21 November 2026 against upstream ELKS
`df2a61c5d72415ff4825aab33be427de9306c230`, with the matching freshly built libc
and IA16 GCC 6.3.0. The build explicitly targets i8086.

- Host tests: PASS.
- Native 16-bit executable build and instruction/near-memory audit: PASS.
- Native tests under elksemu: PASS.
- Staged install and 8.3 destination path checks: PASS.
- Patched upstream `./buildext.sh cmdr scrcap` from fresh public clones: PASS.
- Upstream ExtApplications install selection: PASS.
- EPT recipe build commands and declared source-file checks: PASS.

Direct builds, external builds and the staged executables had identical SHA-256
checksums. The published ZIPs contain the tested executable, license and usage guide.
The full EPT Docker/server publishing cycle was not run; server inclusion is pending.

The emulator test copy uses the isolated LP64 `select` timeval fix supplied by
`tests/prepare_emulator.py` in Cmdr. This changes the host test harness, not the
ELKS application or target kernel. Tests run against temporary files and synthetic
framebuffers; they do not access a palmtop or its batteries.

Cmdr tests cover binary copy, overwrite/self-copy refusal, cancellation, move,
symlink/device handling, empty-only directory deletion, directory limits, key
mappings, interactive navigation/viewing, confirmation, and terminal restoration.
Host fault injection covers short writes, disk-full rollback and failed source
removal after a move copy.

The predecessor ELX Commander was exercised on HP 200LX hardware. This standalone
build has not yet been retested on physical hardware or other ELKS machines.
