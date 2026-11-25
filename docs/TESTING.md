# Development beta 0.4.0-beta1

The beta directly compiles the ELKS TUI import from upstream
`a450d52405b8451fecd2959b769b08462104b398`. The native toolchain remains IA16 GCC
6.3.0 with the previously validated ELKS/libc build at
`df2a61c5d72415ff4825aab33be427de9306c230`; no libc upgrade is required.

- Host file-operation, allocation-failure and PTY UI tests: PASS.
- AddressSanitizer/UndefinedBehaviorSanitizer versions of the host tests: PASS.
- Native 8086 build, instruction audit and near-memory audit: PASS.
- Native file-operation and PTY UI tests under isolated elksemu: PASS.
- Both-panel 600-file coverage and operations beyond entry 128: PASS.
- SS3, VT, CSI and Linux-console function-key sequences delivered in separate
  writes with 25 ms gaps; bare Escape cancellation: PASS.
- Number commands, literal digits in filename prompts, bracketed-paste
  suppression, exit and terminal restoration: PASS.
- 120x40 layout, resize/refresh to 100x30, ANSI DSR size detection, no-response
  fallback and preservation of a typed key during probing: PASS.
- Oversized and malformed DSR parameters fall back safely: PASS.
- External shell and reinitialization: host PASS; this emulator fixture does
  not contain an ELKS `/bin/sh`, so it does not cover native shell return.
- Original imported-file hashes and applying the saved upstream diff back onto
  the pristine import: PASS. Installed names remain 8.3-compatible.

Native executable SHA-256:
`eec61d2f3e9bc32bfabc9dbf3aa56d9044ec14347786790dc59eae0c269e556d`.
The audit reports 61,408 bytes of static data + 48 KiB heap + 4 KiB stack before
arguments/environment, leaving 4,112 bytes in the 65,520-byte data segment.

**Physical PPC and HP beta tests are pending.** The PPC did not respond to FTP
or Telnet during this run, so no beta was transferred or installed. In particular,
the exact macOS/QEMU function-key failure reported in the discussion has not
been reproduced or verified fixed on that setup. The matching escape sequences
are covered by PTY tests, which do not test macOS key interception or QEMU input.

Before promotion, test the beta from a separate directory on the PPC/HP:
80x25 console startup; direct number and function keys; bare Escape; a larger
ANSI Telnet session and refresh after resizing; copy/move/view/delete in a small
temporary fixture; external shell return; exact terminal restoration on quit and
Ctrl-C. Preserve the stable package and its EPT registration while testing.

The stable GitHub/EPT release remains 0.3.0. CP437/Unicode rendering and external
image viewers are deferred. The earlier large-directory PPC performance limit
below has not been resolved by this terminal integration.

# Release 0.3.0

Rebuilt from the merged changes with the final 0.3.0 version string on
21 November 2026. Host tests, native file/UI tests under isolated elksemu,
8086 ISA audit and near-memory audit passed. Staged installed files were packed
into the release ZIP and verified against the native build. The final near-memory
budget is 60,784 bytes before argv/environment.

Release executable SHA-256:
`c9c7d25c2bca8acebf4eccbbe8a9a5f0dad44c4b1a987066b6e7db8e5b355496`.

## EPT upgrade on the physical PPC 512

The final release was published in the stable catalogue at
<http://ept.8088retro.com/> on 21 November 2026. Public HTTP verification passed
for ten endpoints, including exact bytes, Content-Length and no redirects or
content compression.

With that repository selected, `ept update` followed by `ept install cmdr`
upgraded the physical Amstrad PPC 512 running ELKS 0.9.2 from 0.2.0 to 0.3.0.

- EPT exited successfully; all three installed files matched the release hashes
  and expected modes (755 for the executable, 644 for documentation).
- `cmdr --version` reported 0.3.0, and EPT registered all three stable-package
  files at 0.3.0. Other package records and the EPT executable were unchanged.
- The installed `/bin/cmdr` opened over Telnet with all 206 `/bin` entries in
  each panel. End, panel switching and refresh retained the final selections.
- Help and return passed. Normal quit returned status 0; both quit and Ctrl-C
  restored the original terminal settings.
- Original EPT configuration and cached catalogues were restored afterward;
  cmdr 0.3.0 remains installed and registered.

The broader development-build hardware results and unresolved large-directory
performance limitation follow.

# Development validation: 0.3.0-dev

Validated on 21 November 2026 with the same upstream ELKS/IA16 environment
recorded for 0.2.0 below.

- Host file-operation and real-PTY tests: PASS.
- AddressSanitizer and UndefinedBehaviorSanitizer runs of the same tests: PASS.
- Native 8086 build, instruction audit and near-memory audit: PASS.
- Native file-operation and real-PTY tests under isolated elksemu: PASS.
- 600 files plus `..`, complete sorted enumeration and 20 reloads: PASS.
- End/refresh/copy beyond entry 128 without a controlling terminal: PASS.
- Injected allocation failures in array and filename allocation: incomplete
  listing reported, refresh recovers, and storage can be freed safely.
- Existing copy/move/delete safety and terminal-restoration checks: PASS.

The native build uses a 48 KiB heap and 4 KiB stack; the audited static+heap+stack
budget is 60,800 bytes before arguments/environment, below the 65,520-byte
segment budget with a further 1 KiB reserve. Capacity is memory-dependent,
not unlimited. Name/path limits remain unchanged.

`fm` was used as a reference for dynamically allocated listings and working
with inherited terminal descriptors. cmdr keeps its own UI and file operations.
The native allocator is unchanged. Array growth allocates the replacement before
freeing the old array, so allocation failure leaves the collected entries valid.

The test-only elksemu preparation now also fixes two guest-memory overruns:
`fd_set` is 32 bits in ELKS, and Linux `d_reclen` is not a filename length.
Directory conversion copies at most ELKS MAXNAMLEN (26) bytes and terminates the
name. These changes affect the isolated host emulator, not the PPC kernel or
application. The earlier LP64 timeval conversion fix is retained.

## Physical PPC 512 / ELKS 0.9.2

The development executable SHA-256 is
`f7f6c62c372c6ff231e2a6b632bd5cabff5269df1b7b87df74a4f6a3c47008ef`.
Its FTP transfer was verified byte-for-byte before testing.

- Telnet startup with inherited terminal descriptors: PASS.
- Both panels showing all 206 `/bin` entries; End reaches entry 206 and refresh
  retains selection with no incomplete-list warning: PASS.
- Temporary-file viewing, copy cancellation, confirmed copy, overwrite refusal,
  move, mkdir, enter/parent navigation and empty-directory deletion: PASS.
- Destination contents after copy and move verified byte-for-byte by FTP; source
  removed after move, and the temporary directory removed: PASS.
- Help, external shell and return to cmdr: PASS.
- Normal quit returns status 0; `stty` output identical before/after: PASS.
- Ctrl-C exits and restores the same terminal settings: PASS.

A larger hardware fixture was attempted. Extracting 1,200 test files made remote
services slow to respond; the test's `tar` process was stopped. The resulting
590/443-file directories did not reach the first cmdr screen within 120 seconds.
The cause of that delay is not isolated. These are **not** claimed as passing
large-directory hardware tests; 600-entry coverage above is host/emulator only.
A subsequent small-directory Telnet run and the 206-entry tests passed.

At the time of this development test, the stable EPT package remained 0.2.0.
The development binary was staged at `/root/cmdnew21/cmdr` without changing
`/bin/cmdr` or the EPT registry. The final 0.3.0 upgrade is recorded above.

The same development binary also passed physical-console startup with all 206
`/bin` entries and SIGTERM cleanup. The original console launcher and inittab
were verified restored/unchanged after capture. [Console screenshot](images/ppc-dynamic.png).

Upstream `fm` was also captured from the physical console for comparison:
[fm screenshot](images/ppc-fm.png). Both PNGs render actual B8000 video memory
using the previously captured PPC BIOS font; the 200 scanlines are doubled
vertically. Raw character/attribute data is retained alongside each PNG.
They are not terminal mockups.

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
