#ifndef APCI_FOPS_H
#define APCI_FOPS_H

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "apci_common.h"
#include "apci_fops.h"

#define apci_get_devices_ioctl      _IO(ACCES_MAGIC_NUM , 1)
#define apci_get_device_info_ioctl  _IOR(ACCES_MAGIC_NUM, 2, info_struct *)
#define apci_write_ioctl            _IOW(ACCES_MAGIC_NUM, 3, iopack *)
#define apci_read_ioctl             _IOR(ACCES_MAGIC_NUM, 4, iopack *)
#define apci_wait_for_irq_ioctl     _IOR(ACCES_MAGIC_NUM, 5, unsigned long)
#define apci_cancel_wait_ioctl      _IOW(ACCES_MAGIC_NUM, 6, unsigned long)
#define apci_get_base_address       _IOW(ACCES_MAGIC_NUM, 7, unsigned long)


ssize_t read_apci(struct file *f, char __user *buf, size_t len, loff_t *off);
int open_apci( struct inode *inode, struct file *filp );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,16,39)
int ioctl_apci(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else 
long ioctl_apci(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
#endif
