#!/usr/bin/env python3
"""Make an isolated elksemu copy fixing LP64 select and host-to-ELKS directory entry conversion."""
from pathlib import Path
import shutil,subprocess,sys
src=Path(sys.argv[1]).resolve();dst=Path(sys.argv[2]).resolve()
assert src!=dst and not dst.is_relative_to(src)
shutil.copytree(src,dst,dirs_exist_ok=True)
p=dst/'elks_sys.c';s=p.read_text();start=s.index('static void\nsquash_fd_set');end=s.index('#define sys_socket',start)
block=s[start:end]
assert block.count('ELKS_PEEK(long,')==2 and block.count('ELKS_POKE(long,')==2
block=block.replace('ELKS_PEEK(long,','ELKS_PEEK(int32_t,').replace('ELKS_POKE(long,','ELKS_POKE(int32_t,')
# ELKS fd_set is one __u32 (20 descriptors), not a 64-bit host word.
# Copying eight bytes back overwrites the adjacent guest stack variable.
assert block.count('sizeof(uint64_t)')==2
block=block.replace('sizeof(uint64_t)', 'sizeof(uint32_t)')
s=s[:start]+block+s[end:]
# Linux d_reclen is the entire host record length, not the filename length.
# Copying it into ELKS's 27-byte name buffer corrupts the following heap block.
old='''    ELKS_POKE(int16_t, cx + 8, ent->d_reclen);
    memcpy(ELKS_PTR(char, cx + 10), ent->d_name, ent->d_reclen + 1);'''
new='''    size_t len = strlen(ent->d_name);
    if (len > 26) len = 26;
    ELKS_POKE(int16_t, cx + 8, len);
    memcpy(ELKS_PTR(char, cx + 10), ent->d_name, len);
    ELKS_POKE(char, cx + 10 + len, 0);'''
assert old in s
s=s.replace(old,new)
p.write_text(s)
subprocess.run(['make','-C',str(dst),'TOPDIR='+str(src.parent),'PREFIX='+str(src.parent/'cross'),'elksemu'],check=True)
print('Isolated emulator: '+str(dst/'elksemu'))
