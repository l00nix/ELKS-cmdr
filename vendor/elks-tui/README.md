# ELKS TUI import

The five C/header files were imported byte-for-byte from
[ELKS a450d52405b8451fecd2959b769b08462104b398](https://github.com/ghaerr/elks/tree/a450d52405b8451fecd2959b769b08462104b398/elkscmd/tui)
in commit `2ae42da`, before any local edits. `UPSTREAM.json` records the original
hashes. This follows [Ghaerr's recommendation](https://github.com/ghaerr/elks/discussions/2859#discussioncomment-19877552).
Original author and license comments are retained; installed notices are in
`/lib/cmdr/tuilic.txt`.

cmdr uses `tty_init`, `tty_restore`, `tty_getsize`, `readansi` and
`ansi_to_unikey`, translating the Unicode private-use keys to its internal key
codes with unsigned comparisons suitable for 16-bit ints. `runes.c` is compiled
intact; unused routines are removed by the native linker's section collection.
ASCII borders remain in use. No system library is replaced.

## Local changes for review

- `tty.c`: operate on inherited stdin for termios, guard optional `IUTF8`, track
  active state to make restoration idempotent, disable bracketed paste on exit,
  and restore using `TCSAFLUSH` (host PTY tests detected pending-input state with
  `TCSADRAIN`). Report terminal setup failure through `tty_init`.
- `unikey.c`: map ELKS console ESC a..l / ESC [ a..l to function keys only
  when `iselksconsole` is true, preserving remote Alt-letter combinations; add SS3, CSI P–S and Linux-console F1–F5 decoding; consume the full
  Linux sequence; use 200 ms `select` waits to finish bare Escape and bound
  reads/probes, returning zero on idle timeout and errors to the caller. NoWait
  retains zero-wait polling. Timed reads change upstream blocking semantics and
  need maintainer review before
  adoption by other TUI applications. cmdr uses its own signal handlers and
  does not enable mouse or NoWait modes.
- Prefer `TIOCGWINSZ` when available, otherwise probe with DSR. Preserve one
  non-DSR input event during probing for the next `readansi` call; cmdr discards
  late DSR reports rather than interpreting them as commands.
- Bound decimal parsing for 16-bit ints and validate DSR parameter syntax.
- Make the private digit helper static with a distinct name; use explicit
  unsigned key comparisons to compile cleanly on host and IA16 toolchains.
- `runes.c`, `runes.h` and `unikey.h` are unchanged.

Reproduce the patch from the repository root:

```sh
git diff 2ae42da -- vendor/elks-tui/tty.c vendor/elks-tui/unikey.c \
  vendor/elks-tui/runes.c vendor/elks-tui/unikey.h vendor/elks-tui/runes.h
```

The same diff is saved as [integration/elks-tui.patch](../../integration/elks-tui.patch).
It is a review artifact for cmdr's local integration, not a claim of upstream
acceptance or testing with every ELKS compiler. No future library ABI is assumed.
