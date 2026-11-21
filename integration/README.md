# External build and EPT integration

Integration submissions (pending maintainer acceptance):

- [ELKS external build PR #2858](https://github.com/ghaerr/elks/pull/2858).
- [EPT recipe PR #12](https://github.com/sammothxc/ept/pull/12).
- Recipes are also committed to [l00nix/ept](https://github.com/l00nix/ept/tree/main/recipes).

As checked on 21 November 2026, cmdr is absent from both the stable and nightly
catalogues at <http://ept.8088retro.com/>. The EPT PR is open and unmerged;
a recipe submission is not a published binary package. Repository URLs now use
`l00nix/ELKS-cmdr`; the installed command and package name remain `cmdr`.

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
Once a configured EPT repository publishes it, install with `ept install cmdr`.
A GitHub release alone does not add it to EPT's default server.

All installed path components are 8.3-compatible: `/bin/cmdr`,
`/lib/cmdr/copying`, `/lib/cmdr/usage.txt`. Host build filenames are not
installed on ELKS. No compatibility alias using the old executable name is installed.
