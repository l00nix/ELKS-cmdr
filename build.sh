#!/bin/sh
set -eu
: "${ELKS:=${TOPDIR:-}}"
: "${ELKS:?Set ELKS to a configured ELKS source/toolchain}"
cd "$(dirname "$0")"
CROSS=${CROSS:-${CROSSDIR:-$ELKS/cross}}
BUILD=${BUILD:-build}
case "$BUILD" in /*) ;; *) BUILD="$(pwd)/$BUILD";; esac
mkdir -p "$BUILD/host-tools" "$BUILD/obj"
cp toolchain/retain-elf2elks.py "$BUILD/host-tools/elf2elks"
chmod +x "$BUILD/host-tools/elf2elks"
ELX_REAL_ELF2ELKS=$ELKS/elks/tools/bin/elf2elks
PATH=$BUILD/host-tools:$CROSS/bin:$ELKS/elks/tools/bin:$PATH
export PATH ELX_REAL_ELF2ELKS
CC=${CC:-$CROSS/bin/ia16-elf-gcc}
if [ ! -x "$CC" ]; then CC=$(command -v ia16-elf-gcc); fi
SIZE=${SIZE:-$(command -v ia16-elf-size)}
OBJDUMP=${OBJDUMP:-$(command -v ia16-elf-objdump)}
flags="-Os -melks-libc -march=i8086 -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror -Wno-misleading-indentation -ffunction-sections -fdata-sections -fstack-usage"

for name in main files terminal; do
    "$CC" $flags -Isrc -I"$ELKS/include" -I"$ELKS/libc/include" -I"$ELKS/elks/include" -c "src/$name.c" -o "$BUILD/obj/$name.o"
done
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/cmdr.map" -maout-heap=49152 -maout-stack=4096 \
    "$BUILD/obj/main.o" "$BUILD/obj/files.o" "$BUILD/obj/terminal.o" -o "$BUILD/cmdr"
"$SIZE" "$BUILD/cmdr.elf"
"$OBJDUMP" -d -z --insn-width=16 -M intel,i8086 "$BUILD/cmdr.elf" > "$BUILD/cmdr.dis"
python3 toolchain/isa-audit.py "$BUILD/cmdr" "$BUILD/cmdr.elf" "$BUILD/cmdr.dis" --map "$BUILD/cmdr.map" --output "$BUILD/cmdr-audit.json"
"$CC" --version > "$BUILD/compiler-version.txt"
"$CC" $flags -Isrc -I"$ELKS/include" -I"$ELKS/libc/include" -I"$ELKS/elks/include" -Wl,--gc-sections -maout-heap=49152 -maout-stack=4096 \
    tests/ops.c src/files.c src/terminal.c -o "$BUILD/test-ops"
