# Cmdr

A small two-panel, 80x25 ANSI file manager for [ELKS](https://github.com/ghaerr/elks).
The executable is **`cmdr`** (formerly `elxcmd` / ELX Commander).
It uses terminal interfaces rather than HP firmware, so it can run on other ELKS
machines with a suitable console or serial terminal.

## Use

```sh
TERM=ansi cmdr /root /tmp
```

| Keys | Action |
| --- | --- |
| Tab | Switch panel |
| Arrows, Home/End, PgUp/PgDn | Select and scroll |
| Enter | Open a directory |
| F1 / ? | Help |
| F2 / P | Change panel path |
| F3 / V | View a file; Q returns to panels |
| F4 / E | Edit using vi |
| F5 / C | Copy selected file to other panel |
| F6 / M | Move selected file to other panel |
| F7 / N | Create a directory |
| F8 / D | Delete a file or empty directory |
| F9 / ! | Shell in active directory |
| R | Refresh listings |
| F10 / Q | Quit |

Copy, move and delete require confirmation. Copy/move refuse existing destinations.
Directory trees are not copied or moved recursively. Listings hold up to 128 entries
per panel and report truncation. The UI is fixed at 80x25. No mouse or Nano-X is required.

## Build and install

Use a configured [ELKS source tree](https://github.com/ghaerr/elks) with its
IA16 compiler and matching libc installed:

```sh
ELKS=/path/to/elks sh build.sh
make install DESTDIR=/path/to/staging-root
```

The build targets the 8086 and audits the retained ELF, disassembly and near-memory
layout. Output is `build/cmdr`. Install paths are `/bin/cmdr` and
`/lib/cmdr/{copying,usage.txt}`; every installed component fits 8.3 naming.
The staging root must exist on the host; do not point it at an unintended filesystem.

[External build and EPT integration](integration/README.md) includes the package
recipe. Upstream registration and EPT server publication are separate steps;
the recipe alone does not make the package available from the default EPT server.

## Tests and status

```sh
make test
```

Host tests require a C compiler and Python 3. On macOS, Cmdr's legacy formatted
output calls may need `CFLAGS='-O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Wno-deprecated-declarations -Isrc'`.
Native builds are additionally checked with `elksemu`; synthetic memory fixtures
are not hardware captures. See [validation](docs/TESTING.md) for this release.

## Origins and license

Developed by Alexander Rau with AI assistance, originally as part of the
[HP 200LX ELX suite](https://github.com/l00nix/HP-200LX-elx-suite).
The standalone source starts from suite commit `a2cc3f3`; historical suite releases
retain their original names. ELKS provides the operating system, libc and toolchain.
Project code is [GPL-2.0-or-later](COPYING). Existing attribution is retained.

Inspired by Norton Commander and classic two-panel file managers; no Norton source
or binaries are included.
