#!/bin/sh

set -e

# warn user about various smep/smap options 
echo "[!] dont forget to change qemu smep/smap options!"
echo "[~] observe stack smashing: just \`break *0xffffffffc0000057\` in \`gdb\`"

# prepare rootfs and compile exploits
echo "[+] building exploits"
set -x
gcc ./sources/exploits/smap.c -o smap --static -Wall -Werror
mv ./smap ./sources/initramfs/exp/.
gcc ./sources/exploits/smep.c -o smep --static -Wall -Werror
mv ./smep ./sources/initramfs/exp/.
gcc ./sources/exploits/nothing.c -o nothing --static -Wall -Werror
mv ./nothing ./sources/initramfs/exp/.
set +x
echo "[+] building rootfs"
set -x
cd ./sources/initramfs && find . | cpio -o -H newc -R root:root | gzip -9 > ./../../initramfs.cpio.gz && cd -
set +x

# run VM
echo "[+] starting VM"
qemu-system-x86_64  \
-m 2048M \
-kernel ./bzImage \
-s \
-initrd initramfs.cpio.gz \
-nographic \
-smp 1,cores=1,threads=1 \
-cpu qemu64,+smep,+smap \
-append "console=ttyS0 pti=off quiet nosmp nopti nokaslr"
# +smep states for `smep on`, -smep states for `smep off`

# clean
echo "\n[+] cleaning"
set -x
rm initramfs.cpio.gz ./sources/initramfs/exp/nothing ./sources/initramfs/exp/smep ./sources/initramfs/exp/smap
