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
The original EPT Docker/server publishing cycle was not run; inclusion on that
server is pending. Independent 8088retro publication and PPC installation are
recorded below.

The emulator test copy uses the isolated LP64 `select` timeval fix supplied by
`tests/prepare_emulator.py` in Cmdr. This changes the host test harness, not the
ELKS application or target kernel. Tests run against temporary files and synthetic
framebuffers; they do not access a palmtop or its batteries.

Cmdr tests cover binary copy, overwrite/self-copy refusal, cancellation, move,
symlink/device handling, empty-only directory deletion, directory limits, key
mappings, interactive navigation/viewing, confirmation, and terminal restoration.
Host fault injection covers short writes, disk-full rollback and failed source
removal after a move copy.

The predecessor ELX Commander was exercised on HP 200LX hardware. On
21 November 2026, the standalone 0.2.0 release binary was also exercised on a
physical Amstrad PPC 512 running ELKS 0.9.2: startup, `/etc`, `/bin` and `/lib`
listings, the 128-entry warning, and SIGTERM console cleanup. The binary transfer
was verified byte-for-byte; the original console launcher was restored afterward.
[Hardware screenshots and reproducible capture details](images/README.md) are
included. This limited smoke test does not cover physical-keyboard interaction,
copy/move/delete on the PPC, or retesting the standalone binary on the HP.

## EPT publication and physical PPC installation — 21 November 2026

- cmdr 0.2.0 was published in the stable catalogue at
  <http://ept.8088retro.com/index>, using the unmodified release ZIP.
- The public index, package files and source archive returned exact expected
  bytes with Content-Length, no redirects and no content compression.
- On a physical Amstrad PPC 512 running ELKS 0.9.2, `ept install cmdr` downloaded
  the package from that public repository and exited successfully.
- `/bin/cmdr`, `/lib/cmdr/copying` and `/lib/cmdr/usage.txt` were read back and
  matched the package SHA-256 hashes. Modes were 755, 644 and 644 respectively.
- EPT registered cmdr 0.2.0 while preserving unrelated installed-package records.
- `cmdr --version` passed over Telnet. Full-screen startup in that Telnet session
  failed with "Cmdr: needs an 80x25 text terminal: No such device"; interactive
  Telnet operation is not validated.
- The EPT-installed `/bin/cmdr` also started on the physical console. A concurrent
  scrcap test wrote to the same display, so this run is not presented as a clean
  screenshot. Earlier physical-console captures use this identical binary.
- The original `/etc/ept.cfg` and cached catalogues were restored afterward;
  the installed cmdr files and its EPT registration were retained.

This installation check supplements the earlier physical-console screenshots.
It does not claim a new copy/move/delete regression test on hardware.
