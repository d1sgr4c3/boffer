#!/bin/sh
set -e


get_choice() {
    local prompt="$1"
    local choice
    while true; do
        read -p "$prompt (y/N): " choice
        case "$choice" in
            y|Y ) echo true; return;;
            n|N ) echo false; return;;
	     "" ) echo false; return;; 
            * ) echo "type your choice [y/N]: ";;
        esac
    done
}

# warn user about various smep/smap options 
echo "[!] dont forget to change qemu smep/smap options!"
echo "[~] observe stack smashing: just \`break *0xffffffffc0000057\` in \`gdb\`"

ask_smep=$(get_choice "enable SMEP ?")
ask_smap=$(get_choice "enable SMAP ?")
choise_smep=""
choise_smap=""

if [ "$ask_smep" = true ]; then
    choise_smep+="+smep"
else
    choise_smep+="-smep"
fi
if [ "$ask_smap" = true ]; then
    choise_smap+="+smap"
else
    choise_smap+="-smap"
fi

# prepare rootfs and compile exploits
echo "[+] preparing rootfs"
set -x
mkdir initramfs
pushd . && pushd initramfs
cp ../initramfs.cpio.gz .
gzip -dc initramfs.cpio.gz | cpio -idm &>/dev/null && rm initramfs.cpio.gz
popd
set +x
echo "[+] building exploits"
set -x
gcc ./smap.c -o smap --static -Wall -Werror
mv ./smap ./initramfs/exp/.
gcc ./smep.c -o smep --static -Wall -Werror
mv ./smep ./initramfs/exp/.
gcc ./nothing.c -o nothing --static -Wall -Werror
mv ./nothing ./initramfs/exp/.
set +x
echo "[+] building rootfs"
set -x
cd ./initramfs && find . | cpio -o -H newc -R root:root | gzip -9 > ./../initramfs.cpio.gz && cd -
rm -rf ./initramfs
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
-cpu qemu64,$choise_smep,$choise_smap \
-append "console=ttyS0 pti=off quiet nosmp nopti nokaslr"
# +smep states for `smep on`, -smep states for `smep off`
