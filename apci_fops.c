#include "apci_fops.h"

ssize_t read_apci(struct file *f, char __user *buf,
                         size_t len, loff_t *off)
{
    size_t i = min_t(size_t, len, 4);
    return copy_to_user(buf, APCI "\n", i) ? -EFAULT : i;
}
