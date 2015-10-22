#ifndef APCI_IOCTL_H


#define APCI_IOCTL_H
#define ACCES_MAGIC_NUM 0xE0
#define A_VENDOR_ID 0x494F


enum SIZE { BYTE = 0, WORD, DWORD};

#ifndef __u32 
#define __u32 unsigned int
#endif

typedef struct {
        unsigned long device_index;
        int bar;
        unsigned int offset;
        enum SIZE size;
        __u32 data;
} iopack;

typedef struct {
        unsigned long device_index;
        int dev_id;
        /* Most likely only base_addresses[2] will be useful.
         * Consult your product manual for more information.
         */
        unsigned long base_addresses[6];
} info_struct;

#ifdef __KERNEL__
#include <linux/ioctl.h>


#else
#include <sys/ioctl.h>
#endif

#define apci_get_devices_ioctl		_IO(ACCES_MAGIC_NUM , 1)
#define apci_get_device_info_ioctl	_IOR(ACCES_MAGIC_NUM, 2, info_struct *)
#define apci_write_ioctl		_IOW(ACCES_MAGIC_NUM, 3, iopack *)
#define apci_read_ioctl			_IOR(ACCES_MAGIC_NUM, 4, iopack *)
#define apci_wait_for_irq_ioctl		_IOR(ACCES_MAGIC_NUM, 5, unsigned long)
#define apci_cancel_wait_ioctl		_IOW(ACCES_MAGIC_NUM, 6, unsigned long)
#define apci_get_base_address		_IOW(ACCES_MAGIC_NUM, 7, unsigned long)

#define apci_inportb_ioctl		_IOR(ACCES_MAGIC_NUM, 8 , unsigned char)
#define apci_inport_ioctl		_IOR(ACCES_MAGIC_NUM, 9 , _u16 )
#define apci_inportdw_ioctl		_IOR(ACCES_MAGIC_NUM, 10, unsigned int)
/* #define apci_inportl apci_inportdw  */

#define apci_relinportb_ioctl		_IOR(ACCES_MAGIC_NUM, 11 , unsigned char)
#define apci_relinport_ioctl		_IOR(ACCES_MAGIC_NUM, 12 , _u16 )
#define apci_relinportdw_ioctl		_IOR(ACCES_MAGIC_NUM, 13, unsigned int)

#endif
