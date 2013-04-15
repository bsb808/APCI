#include "apci_fops.h"
#include "apci_dev.h"

int open_apci( struct inode *inode, struct file *filp )
{
  struct apci_my_info *ddata;
  apci_debug("Opening device\n");
  ddata = container_of( inode->i_cdev, struct apci_my_info, cdev );
  /* need to check to see if the device is 
     Blocking  / nonblocking */

  filp->private_data = ddata;
  apci_debug("Done opening device\n");
  return 0;
}


ssize_t read_apci(struct file *filp, char __user *buf,
                         size_t len, loff_t *off)
{
    size_t i = min_t(size_t, len, 4);
    /* struct apci_my_info  *ddata = filp->private_data ; */

    return copy_to_user(buf, APCI "\n", i) ? -EFAULT : i;
}
