#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static int vuln_init(void);
static void vuln_exit(void);

MODULE_LICENSE("GPL");
static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static ssize_t vuln_read(struct file *file, char *buf, size_t count,
                         loff_t *f_pos) {
  return -EPERM;
}

static ssize_t vuln_write(struct file *file, const char *buf, size_t count,
                          loff_t *f_pos) {
  unsigned long *ptr;
  char buffer[32];
  memset(buffer, 0, 32);
  copy_from_user_nofault(buffer, buf, count);
  /* copy_from_user_nofault() is an unsafe function,
   * which don't checks capacity of target buffer.
   * copy_from_user() mitigates vulnerability
   * by comparing `count` and size of target buffer */
  ptr = (unsigned long *)buffer;
  return *ptr;
}

static int vuln_open(struct inode *inode, struct file *file) { return 0; }

static int vuln_close(struct inode *inode, struct file *file) { return 0; }

static struct file_operations fileops = {
  owner : THIS_MODULE,
  open : vuln_open,
  read : vuln_read,
  write : vuln_write,
  release : vuln_close,
};

int vuln_init(void) {
  alloc_chrdev_region(&first, 0, 1, "vuln");
  cl = class_create(THIS_MODULE, "chardev");
  device_create(cl, NULL, first, NULL, "vuln");
  cdev_init(&c_dev, &fileops);
  cdev_add(&c_dev, first, 1);
  printk(KERN_INFO "Vuln module started\n");
  return 0;
}

void vuln_exit(void) {
  cdev_del(&c_dev);
  device_destroy(cl, first);
  class_destroy(cl);
  unregister_chrdev_region(first, 1);
  printk(KERN_INFO "Vuln module stopped??\n");
}

module_init(vuln_init);
module_exit(vuln_exit);
