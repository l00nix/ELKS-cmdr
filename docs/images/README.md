# Screenshot provenance

## Earlier HP 200LX capture

ELX Commander 0.1 on the double-speed HP 200LX after a successful file copy,
captured with ELXcap on 19 October 2026.

- `cmdr.png`: existing lossless PNG viewing conversion, copied without changes.
- `cmdr.bmp`: original 640×200, 1-bit BMP retrieved from the HP.
- No cropping, resampling, retouching or changes to the displayed application names.
- Transfer checksums were verified against the HP files.
- This is a historical hardware capture, not a claim of hardware validation of the
  renamed 0.2.0 standalone binary.

Original BMP SHA-256: `b7e2bc5a3491744f13acf2b5002e8df89307c70e9f2599402ae9ebc8f9ff3859`.

The files were retained from the HP 200LX project hardware test records for
19 October 2026. They contain the original tool output, not synthetic/demo data.

## Amstrad PPC 512 — 21 November 2026

Cmdr 0.2.0 running on the physical PPC 512, with ELKS 0.9.2 (kernel build
15 November 2026). The executable is identical to `bin/cmdr` in the published
`cmdr-0.2.0-elks.zip` release, SHA-256:
`19c902d6da6c449a5534c66f56542b31a0d8e9f6c199e2fa5653a1b3e936b458`.

- `ppc-etc.png`: actual `/etc` and `/lib` directory listings.
- `ppc-panels.png`: actual `/bin` and `/lib` listings. `/bin` exceeds the current
  128-entry limit, so the application reports an incomplete list.
- The corresponding `.bin` files contain the 4,000 character/attribute bytes
  read from physical video memory at `0xB8000` while the application was running.
- BIOS data area confirmed mode 3, 80 columns, active page 0.
- `ppc-font.bin` contains 128 8x8 glyphs read from this PPC's BIOS at `0xFFA6E`.
- `render-ppc.py` renders those captured cells and glyphs with the standard CGA
  palette, doubling scan lines vertically to 640x400. It does not fabricate text,
  replace filenames, retouch application output, or simulate a terminal session.
  The PNG represents the digital text display; it is not a photograph or a
  simulation of the LCD panel's contrast or colour response.

Reproduce either image with Python 3 (standard library only):

```sh
python3 render-ppc.py ppc-etc.bin ppc-font.bin ppc-etc.png
python3 render-ppc.py ppc-panels.bin ppc-font.bin ppc-panels.png
```

The binary was uploaded to an isolated test directory and read back byte-for-byte.
The temporary console launcher was restored and the machine returned to its
normal console login. No kernel, permanent startup configuration, or EPT package
installation was changed. This verifies startup, real directory display, the
entry-limit warning and SIGTERM cleanup; it is not a full hardware regression
or a physical-keyboard/file-operation test.

### Capture SHA-256 hashes

- `ppc-etc.bin`: `4240f82a7b0d7d1c8c87173af6c9a38597d821c25f6e010d3840199318486650`
- `ppc-panels.bin`: `5d0e9ed53f250d01cc1267713ea447f9ab72c6ceec15158da3fb7bb7a10d5626`
- `ppc-font.bin`: `ddad441c6fd10808a4e94040609cd5471704eb59d18c5fcf6059c028fa8ccae6`
