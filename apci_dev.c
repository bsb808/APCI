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
#include "apci_dev.h"
#include "apci_fops.h"

/* PCI table construction */
static struct pci_device_id ids[] = {
        { PCI_DEVICE(A_VENDOR_ID, PCIe_IIRO_8), },
        { PCI_DEVICE(A_VENDOR_ID, PCI_DIO_24D), },
        {0,}
};
MODULE_DEVICE_TABLE(pci, ids);

static struct class *class_apci;
static struct device *dev_iiro8;
static struct device *dev_dio;

/* PCI Driver setup */
static struct pci_driver pci_driver = {
        .name      = "acpi",
        .id_table  = ids,
        .probe     = probe,
        .remove    = remove
};


/* File Operations */
static struct file_operations apci_fops = { 
        .read = read_apci
};

static const int NUM_DEVICES         = 4;
static const char APCI_CLASS_NAME[]  = "apci";
struct cdev apci_cdev;
static int major_num;

struct apci_my_info head;


static void remove(struct pci_dev *dev)
{
        apci_debug("entering remove");

        delete_drivers( dev );

        apci_debug("Leaving remove");
}


void *
init_new_driver(struct pci_dev *dev, const struct pci_device_id *id ) 
{
 
        struct apci_my_info *ddata = kmalloc( sizeof( struct my_info ) , GFP_KERNEL );
        int count, plx_bar;
        struct resource *presource;

        if( !ddata )
                return NULL;

        /* Fill up the struct apci_device_info_structure structure for this card with starting data. */
        ddata->irq = 0;
        ddata->irq_capable = 0;

        for (count = 0; count < 6; count++) {
                ddata->regions[count].start = 0;
                ddata->regions[count].end = 0;
                ddata->regions[count].flags = 0;
                ddata->regions[count].mapped_address = NULL;
                ddata->regions[count].length = 0;
        }

        ddata->plx_region.start = 0;
        ddata->plx_region.end = 0;
        ddata->plx_region.length = 0;
        ddata->plx_region.mapped_address = NULL;

        ddata->dev_id = id->device;

        spin_lock_init(&(ddata->irq_lock));
        ddata->next = NULL;

        if (pci_resource_flags(dev, 0) & IORESOURCE_IO) {
                plx_bar = 0;
        } else {
                plx_bar = 1;
        }

        ddata->plx_region.start	 = pci_resource_start(dev, plx_bar);
        ddata->plx_region.end	 = pci_resource_end(dev, plx_bar);
        ddata->plx_region.length = ddata->plx_region.end - ddata->plx_region.start + 1;

        apci_debug("plx_region.start = %08x\n", ddata->plx_region.start );
        apci_debug("plx_region.end   = %08x\n", ddata->plx_region.end );
        apci_debug("plx_region.length= %08x\n", ddata->plx_region.length );

        /* TODO: request and remap the region for plx */

        presource = request_region(ddata->plx_region.start,
                                   ddata->plx_region.length, 
                                   "apci");
        if (presource == NULL) {
                /* We couldn't get the region.  We have only allocated
                 * driver_data so release it and return an error.
                 */
          apci_error("Unable to request region.\n");
          kfree(ddata);
          ddata = NULL;
        }


        return ddata;
}


/**
 * @desc Removes all of the drivers from this module
 *       before cleanup
 */
void 
delete_drivers(struct pci_dev *dev)
{
        struct apci_my_info *tmp;
        struct apci_device_info_structure *driver_data, *current_dev;

        apci_debug("Emptying the list of drivers.\n");

        if( driver_data == NULL ) 
          return;

        while (  !list_empty( &head.driver_list ) ) {
                tmp = list_entry( head.driver_list.next , 
                                  struct apci_my_info, 
                                  driver_list );

                release_regions( driver_data->plx_region.start, driver_data->plx_region.length );

                list_del( head.driver_list.next );
                kfree(tmp);
        }
        apci_debug("Completed emptying list.\n");

}


static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
        struct apci_my_info *driver_data;
        apci_debug("entering probe");
    
        if( pci_enable_device(dev) ) {
                return -ENODEV;
        }
        driver_data = (struct apci_my_info*) init_new_driver( dev, id  );

        if( driver_data == NULL ) {
                return -ENOMEM;
        }
        apci_debug("Adding driver to list");
        list_add_tail( &driver_data->driver_list, &head.driver_list );
        apci_debug("Added driver to list");
        return 0;
}

 
int __init 
apci_init(void)
{
	void *ptr_err;
        int result;
        dev_t dev = MKDEV(0,0);
        apci_debug("performing init duties");
        INIT_LIST_HEAD( &head.driver_list );
     
        /* Register memory i/o */
	if ((major_num = register_chrdev(0, APCI_CLASS_NAME , &apci_fops)) < 0)
		return major_num;
        
	class_apci = class_create(THIS_MODULE, APCI_CLASS_NAME  );
	if (IS_ERR(ptr_err = class_apci))
		goto err;

        cdev_init( &apci_cdev, &apci_fops );
        apci_cdev.owner = THIS_MODULE;
        apci_cdev.ops   = &apci_fops;
        result = cdev_add(&apci_cdev, dev, 1 );

#ifdef TEST_CDEV_ADD_FAIL
        result = -1;
        if( result == -1 ) {
                apci_error("Going to delete device.\n");
                cdev_del(&apci_cdev);
                apci_error("Deleted device.\n");
        }
#endif

        if( result < 0 ) {
                apci_error("cdev_add failed in apci_init");
                goto err;
        }
        
        /* needed to get the probe and remove to be called */
        pci_register_driver(&pci_driver);

        /* Goal is to loop through each device that we 
           recognize and print it out 
        */

	return 0;
err:
	/* unregister_chrdev(major_num, APCI_CLASS_NAME); 
           This one is WRONG!!!
        */
        apci_error("Unregistering chrdev_region.\n");
        unregister_chrdev_region(MKDEV(major_num,0),1 );
	class_destroy(class_apci);
	return PTR_ERR(ptr_err);
}

static void __exit apci_exit(void)
{
apci_debug("performing exit duties");
/* order should be 
   pci_unregister_driver
   cdev_del
   unregister_chrdev_region
*/
pci_unregister_driver(&pci_driver);
cdev_del(&apci_cdev);
unregister_chrdev_region(MKDEV(major_num,0),1 );
class_destroy(class_apci );

}

module_init( apci_init  );
module_exit( apci_exit  );

MODULE_AUTHOR("Jimi Damon <jdamon@accesio.com>");
MODULE_LICENSE("GPL");
