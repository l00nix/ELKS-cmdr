# External build and EPT integration

Integration submissions (pending maintainer acceptance):

- [ELKS external build PR #2858](https://github.com/ghaerr/elks/pull/2858).
- [EPT recipe PR #12](https://github.com/sammothxc/ept/pull/12).
- Recipes are also committed to [l00nix/ept](https://github.com/l00nix/ept/tree/main/recipes).

Published on 21 November 2026: cmdr 0.2.0 is available in the stable catalogue
at <http://ept.8088retro.com/>. Set `REPO=http://ept.8088retro.com` in
`/etc/ept.cfg`, then run `ept install cmdr` (no `-n`). See the
[complete installation instructions](../README.md#install-with-ept).

Installation using EPT on the physical PPC 512 passed: all three installed files
matched the published package's SHA-256 hashes, existing package records were
preserved. `cmdr --version` worked over Telnet; full-screen startup in that
session failed with "No such device". The installed binary ran successfully on
the physical console; earlier clean screenshots use the identical release binary. The original EPT configuration
and cached catalogues were restored after testing; cmdr remains installed.
The upstream EPT PR is still open and unmerged. Publication on 8088retro does not
mean the original EPT server has deployed the recipe. Repository URLs use
`l00nix/ELKS-cmdr`; the executable and package name remain `cmdr`.

The [companion patch](elks-apps.patch) adds both applications to `buildext.sh` and
`elkscmd/ExtApplications`. It was tested against ELKS `df2a61c5d72415ff4825aab33be427de9306c230`.
Apply from a clean ELKS checkout with `git am /path/to/elks-apps.patch`, then run
`./buildext.sh cmdr scrcap`. Do not apply again once the upstream change is present.

From a configured ELKS tree, build this checkout with:

```sh
ELKS=/path/to/elks sh build.sh
```

Output: `build/cmdr`. The script targets i8086, retains ELF/map/disassembly,
and runs the ISA/near-memory audit. The compiler and matching ELKS libc must
already be installed. `CROSS` or `CROSSDIR` selects the toolchain prefix; a
compiler on PATH is also supported. No target kernel is modified.

For ghaerr/elks, register `cmdr` in `buildext.sh`, install the binary/docs using `elkscmd/ExtApplications`.
The app source is cloned into `extapps/cmdr`. No floppy-size tags are added; HD images install built external apps.
For custom images use `CONFIG_APP_TAGS=":cmdr|"` (combine tags with `|` and retain the final `|` for ELKS Make.install).

For EPT, copy `cmdr.pkg` into the server recipe directory and rebuild the index.
Tagged releases provide stable packages; the branch can provide nightlies.
The published 8088retro package installs with `ept install cmdr`.
A GitHub release alone does not add it to EPT's default server.

All installed path components are 8.3-compatible: `/bin/cmdr`,
`/lib/cmdr/copying`, `/lib/cmdr/usage.txt`. Host build filenames are not
installed on ELKS. No compatibility alias using the old executable name is installed.
