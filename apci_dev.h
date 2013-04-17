#ifndef APCI_DEV_H
#define APCI_DEV_H


#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <asm/uaccess.h>


#include "apci_common.h"


/* The vendor ID for all the PCI cards this driver will support. */
#define A_VENDOR_ID 0x494F

#define ACCES_MAJOR  98


/* The device IDs for all the PCI cards this driver will support. */
#define PCIe_DIO_24	0x0C52
#define PCIe_DIO_24D	0x0C53
#define PCIe_DIO_24S	0x0E53
#define PCIe_DIO_24DS	0x0E54
#define PCIe_DIO_24DC	0x0E55
#define PCIe_DIO_24DCS	0x0E56
#define PCIe_DIO_48	0x0C61
#define PCIe_DIO_48S	0x0E61
#define PCIe_IIRO_8	0x0F02
#define PCIe_IIRO_16	0x0F09
#define PCI_DIO_24H	0x0C50
#define PCI_DIO_24D	0x0C51
#define PCI_DIO_24H_C	0x0E51
#define PCI_DIO_24D_C	0x0E52
#define PCI_DIO_24S	0x0E50
#define PCI_DIO_48	0x0C60
#define PCI_DIO_48S	0x0E60
#define P104_DIO_48S	0x0E62
#define PCI_DIO_48H	0x0C60
#define PCI_DIO_48HS	0x0E60
#define PCI_DIO_72	0x0C68
#define P104_DIO_96	0x0C69
#define PCI_DIO_96	0x0C70
#define PCI_DIO_96CT	0x2C50
#define PCI_DIO_96C3	0x2C58
#define PCI_DIO_120	0x0C78
#define PCI_AI12_16	0xACA8
#define PCI_AI12_16A	0xACA9
#define PCI_AIO12_16	0xECA8
#define PCI_A12_16A	0xECAA
#define LPCI_A16_16A	0xECE8
#define PCI_DA12_16	0x6CB0
#define PCI_DA12_8	0x6CA8
#define PCI_DA12_6	0x6CA0
#define PCI_DA12_4	0x6C98
#define PCI_DA12_2	0x6C90
#define PCI_DA12_16V	0x6CB1
#define PCI_DA12_8V	0x6CA9
#define LPCI_IIRO_8	0x0F01
#define PCI_IIRO_8	0x0F00
#define PCI_IIRO_16	0x0F08
#define PCI_IDI_48	0x0920
#define PCI_IDO_48	0x0520
#define PCI_IDIO_16	0x0DC8
#define PCI_WDG_2S	0x1250 //TODO: Find out if the Watch Dog Cards should really be here
#define PCI_WDG_CSM	0x22C0
#define PCI_WDG_IMPAC	0x12D0


typedef struct {
    __u32 start;
    __u32 end;
    __u32 length;
    unsigned long flags;
    void *mapped_address;
} io_region;

struct apci_board {
    struct device *dev;
};


  struct apci_my_info {
      __u32 dev_id;
      io_region regions[6], plx_region;
      int irq;
      int irq_capable; /* is the card even able to generate irqs? */
      int waiting_for_irq; /* boolean for if the user has requested an IRQ */
      int irq_cancelled; /* boolean for if the user has cancelled the wait */
      struct list_head driver_list;
      spinlock_t list_lock;
      wait_queue_head_t wait_queue;
      spinlock_t irq_lock;
      struct apci_device_info_structure *next;
      struct cdev cdev;

      struct pci_dev *pci_dev;

      int nchannels;
      struct apci_board boards[APCI_NCHANNELS];

      struct device *dev;
  };


int probe(struct pci_dev *dev, const struct pci_device_id *id);
void remove(struct pci_dev *dev);
void delete_driver(struct pci_dev *dev);



#endif
