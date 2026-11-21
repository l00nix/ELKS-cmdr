# cmdr

[Download 0.2.0](https://github.com/l00nix/ELKS-cmdr/releases/tag/v0.2.0) · [Integration status](integration/README.md) · [Discussions](https://github.com/l00nix/ELKS-cmdr/discussions)

A small two-panel, 80x25 ANSI file manager for [ELKS](https://github.com/ghaerr/elks).
The executable is **`cmdr`** (formerly `elxcmd` / ELX Commander).
It uses terminal interfaces rather than HP firmware, so it can run on other ELKS
machines with a suitable console or serial terminal.

## Development changes (0.3.0-dev)

Directory panels now grow dynamically, with compact filename storage instead of
128 fixed slots. Capacity depends on available memory; an allocation failure or
unreadable/unsupported entry produces an explicit **INCOMPLETE LIST** warning.
The native build reserves a 48 KiB heap within its audited 64 KiB data segment.

Terminal input now uses the inherited stdin descriptor and `select()`, removing
the dependency on opening `/dev/tty` that prevented PPC Telnet startup.
Verified over Telnet on a physical PPC 512: both panels list all 206 `/bin`
entries, file operations and external-shell return work, and quit/Ctrl-C restore
the terminal. Host/emulator tests also cover 600-file directories; larger PPC
fixtures remain slow. See [validation details](docs/TESTING.md).
The stable EPT package below remains 0.2.0 until a new release is published.

## Screenshots

Cmdr 0.2.0 running on a physical Amstrad PPC 512 with ELKS 0.9.2,
captured on 21 November 2026. These images render the actual 80x25 console
video memory using the BIOS font read from the same machine. Scan lines are
doubled vertically for readability; they are not emulator or terminal mockups.

![Cmdr 0.2.0 on the PPC: system configuration and libraries](docs/images/ppc-etc.png)

![Cmdr 0.2.0 on the PPC: executables and libraries](docs/images/ppc-panels.png)

The second capture demonstrates the explicit incomplete-list warning for
`/bin`, which exceeds the current 128-entry panel limit.
[Raw captures and provenance](docs/images/README.md).

### Earlier HP 200LX capture

![Commander file manager after copying alpha.txt](docs/images/cmdr.png)

ELX Commander 0.1 on the double-speed HP 200LX after a successful file copy,
captured with ELXcap on 19 October 2026.
The original 640×200 pixels are preserved. The displayed names and versions predate
the standalone rename. [Original BMP](docs/images/cmdr.bmp) ·
[Capture details](docs/images/README.md).

## Install with EPT

cmdr 0.2.0 is available in the **stable catalogue** of the
[8088retro EPT repository](http://ept.8088retro.com/).
With ELKS networking running and EPT installed, set the `REPO` line in
`/etc/ept.cfg` to the following (keep a copy of your previous configuration):

```ini
REPO=http://ept.8088retro.com
```

Then run as root:

```sh
ept update
ept install cmdr
TERM=ansi cmdr /etc /lib
```

Use plain `http://` for this vintage-client repository. No `-n` option is needed.
The package installs `/bin/cmdr`, `/lib/cmdr/copying` and `/lib/cmdr/usage.txt`.
The installation and all three file hashes were verified on a physical
Amstrad PPC 512 running ELKS 0.9.2. Run cmdr on the local console: the tested
PPC Telnet session could install the package and run `cmdr --version`, but
full-screen startup failed with "No such device". The EPT-installed binary
was separately launched on the physical console.

If EPT is not installed yet, follow the repository's
[EPT bootstrap instructions](http://ept.8088retro.com/).
The [upstream EPT recipe PR](https://github.com/sammothxc/ept/pull/12) is separate
from this published 8088retro package and is still awaiting maintainer acceptance.

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
Directory trees are not copied or moved recursively. Listings grow with available memory and report incomplete results. The UI is fixed at 80x25. No mouse or Nano-X is required.

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
recipe. The 8088retro package is published; registration and deployment on the
original EPT server remain separate maintainer steps.

## Tests and status

```sh
make test
```

Host tests require a C compiler and Python 3. On macOS, Cmdr's legacy formatted
output calls may need `CFLAGS='-O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Wno-deprecated-declarations -Isrc'`.
Native builds are additionally checked with `elksemu`; synthetic memory fixtures
are not hardware captures. See [validation](docs/TESTING.md) for this release.

## Origins and license

Developed by [@l00nix](https://github.com/l00nix) with AI assistance, originally as part of the
[HP 200LX ELX suite](https://github.com/l00nix/HP-200LX-elx-suite).
The standalone source starts from suite commit `a2cc3f3`; historical suite releases
retain their original names. ELKS provides the operating system, libc and toolchain.
Project code is [GPL-2.0-or-later](COPYING). Existing attribution is retained.

Inspired by Norton Commander and classic two-panel file managers; no Norton source
or binaries are included.
