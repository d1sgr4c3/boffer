#!/usr/bin/env python3

import sys
import subprocess
import os

def display_help():
    print("Usage: ./run.py [options]")
    print("Options:")
    print("  +smep    Enable SMEP (Supervisor Mode Execution Protection)")
    print("  -smep    Disable SMEP")
    print("  +smap    Enable SMAP (Supervisor Mode Access Protection)")
    print("  -smap    Disable SMAP")
    print("  --help   Display this help message and exit")

def apply_cpu_options(options):
    cpu_options = ["qemu64"]
    for option in options:
        if option in ["+smep", "-smep", "+smap", "-smap"]:
            cpu_options.append(option)
    return ["-cpu", ",".join(cpu_options)]

def main():
    if len(sys.argv) == 1 or "--help" in sys.argv:
        display_help()
        return

    print("[!] don't forget to change qemu smep/smap options!")
    print("[~] observe stack smashing: just `break *0xffffffffc0000057` in `gdb`")

    print("[+] building exploits")
    subprocess.run(["gcc", "./sources/exploits/smap.c", "-o", "smap", "--static", "-Wall", "-Werror"])
    subprocess.run(["mv", "./smap", "./sources/initramfs/exp/"])
    subprocess.run(["gcc", "./sources/exploits/smep.c", "-o", "smep", "--static", "-Wall", "-Werror"])
    subprocess.run(["mv", "./smep", "./sources/initramfs/exp/"])
    subprocess.run(["gcc", "./sources/exploits/nothing.c", "-o", "nothing", "--static", "-Wall", "-Werror"])
    subprocess.run(["mv", "./nothing", "./sources/initramfs/exp/"])

    print("[+] building rootfs")
    os.chdir("./sources/initramfs")
    with subprocess.Popen(["find", ".", "-print0"], stdout=subprocess.PIPE) as find_process:
        with subprocess.Popen(["cpio", "-o", "-H", "newc", "-R", "root:root", "--null"], stdin=find_process.stdout, stdout=subprocess.PIPE) as cpio_process:
            with open("../../initramfs.cpio", "wb") as cpio_file:
                cpio_file.write(cpio_process.stdout.read())
            with subprocess.Popen(["gzip","-f", "-9", "../../initramfs.cpio"], stdout=open("../../initramfs.cpio.gz", "wb")) as gzip_process:
                gzip_process.communicate()
    os.chdir("../../")

    print("[+] starting VM")
    cpu_options = apply_cpu_options([arg for arg in sys.argv[1:] if arg not in ["--help"]])
    qemu_command = [
        "qemu-system-x86_64",
        "-m", "2048M",
        "-kernel", "./bzImage",
        "-initrd", "initramfs.cpio.gz",
        "-s",
        "-nographic",
        "-smp", "1,cores=1,threads=1",
    ] + cpu_options + [
        "-append", "console=ttyS0 pti=off quiet nosmp nopti nokaslr"
    ]
    subprocess.run(qemu_command)

    print("\n[+] cleaning")
    subprocess.run(["rm", "initramfs.cpio.gz", "./sources/initramfs/exp/nothing", "./sources/initramfs/exp/smep", "./sources/initramfs/exp/smap"])

if __name__ == "__main__":
    main()
