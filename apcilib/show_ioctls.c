#include <stdio.h>
#include <asm/ioctl.h>
#include <apci_ioctl.h>



int main (void)
{
  printf("apci_get_devices_ioctl: %lu\n",       _IO(ACCES_MAGIC_NUM , 1));
  printf("apci_get_device_info_ioctl: %lu\n",   _IOR(ACCES_MAGIC_NUM, 2, info_struct *));
  printf("apci_write_ioctl: %lu\n",             _IOW(ACCES_MAGIC_NUM, 3, iopack *));
  printf("apci_read_ioctl: %lu\n",              _IOR(ACCES_MAGIC_NUM, 4, iopack *));
  printf("apci_wait_for_irq_ioctl: %lu\n",      _IOR(ACCES_MAGIC_NUM, 5, unsigned long));
  printf("apci_cancel_wait_ioctl: %lu\n",       _IOW(ACCES_MAGIC_NUM, 6, unsigned long));
  printf("apci_get_base_address: %lu\n",        _IOW(ACCES_MAGIC_NUM, 7, unsigned long));
}
