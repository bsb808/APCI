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
#include <asm/io.h>
#include <asm/uaccess.h>
#include "apci_common.h"

ssize_t read_apci(struct file *f, char __user *buf, size_t len, loff_t *off);
int open_apci( struct inode *inode, struct file *filp );

#endif
