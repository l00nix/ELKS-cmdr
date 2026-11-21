# External build and EPT integration

The upstream integration is proposed separately; these files do not claim inclusion.
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
For custom images use `CONFIG_APP_TAGS=":cmdr"` (combine tags with `|`).

For EPT, copy `cmdr.pkg` into the server recipe directory and rebuild the index.
Tagged releases provide stable packages; the branch can provide nightlies.
Once a configured EPT repository publishes it, install with `ept install cmdr`.
A GitHub release alone does not add it to EPT's default server.

All installed path components are 8.3-compatible: `/bin/cmdr`,
`/lib/cmdr/copying`, `/lib/cmdr/usage.txt`. Host build filenames are not
installed on ELKS. No compatibility alias using the old executable name is installed.
