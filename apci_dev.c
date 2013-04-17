/*
 * ACCES I/O APCI Linux driver 
 *
 * Copyright 1998-2013 Jimi Damon <jdamon@accesio.com>
 *
 */ 
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
#include <linux/idr.h>

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


struct apci_lookup_table_entry {
  int board_id;
  int counter;
  char *name;
};

static struct apci_lookup_table_entry acpi_lookup_table[] = {
  { PCIe_IIRO_8, 0 , "iiro8" } ,
  { PCI_DIO_24D, 0 , "pci24d" },
  {0}
};

int APCI_LOOKUP_FN(int x ) { 
  switch(x) {
  case PCIe_IIRO_8:
    return 0;
  case PCI_DIO_24D:
    return 1;
  default:
    return -1;
  }
}

static struct class *class_apci;

/* PCI Driver setup */
static struct pci_driver pci_driver = {
        .name      = "acpi",
        .id_table  = ids,
        .probe     = probe,
        .remove    = remove
};


/* File Operations */
static struct file_operations apci_fops = { 
  .read = read_apci,
  .open = open_apci
};

static const int NUM_DEVICES         = 4;
static const char APCI_CLASS_NAME[]  = "apci";
struct cdev apci_cdev;
static int major_num;


static struct class *class_apci;
struct apci_my_info head;
static int dev_counter = 0;

static dev_t apci_first_dev = MKDEV(0,0);


void *
apci_alloc_driver(struct pci_dev *pdev, const struct pci_device_id *id ) 
{
 
    struct apci_my_info *ddata = kmalloc( sizeof( struct apci_my_info ) , GFP_KERNEL );
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

    if (pci_resource_flags(pdev, 0) & IORESOURCE_IO) {
        plx_bar = 0;
    } else {
        plx_bar = 1;
    }

    ddata->plx_region.start	 = pci_resource_start(pdev, plx_bar);
    if( ! ddata->plx_region.start ) {
      apci_error("Invalid bar %d on start ", plx_bar );
      goto out_alloc_driver;
    }
    ddata->plx_region.end	 = pci_resource_end(pdev, plx_bar);
    if( ! ddata->plx_region.start ) {
      apci_error("Invalid bar %d on end", plx_bar );
      goto out_alloc_driver;
    }


    ddata->plx_region.length = ddata->plx_region.end - ddata->plx_region.start + 1;

    apci_debug("plx_region.start = %08x\n", ddata->plx_region.start );
    apci_debug("plx_region.end   = %08x\n", ddata->plx_region.end );
    apci_debug("plx_region.length= %08x\n", ddata->plx_region.length );

    /* TODO: request and remap the region for plx */

    presource = request_region(ddata->plx_region.start, ddata->plx_region.length, "apci");
    if (presource == NULL) {
        /* We couldn't get the region.  We have only allocated
         * driver_data so release it and return an error.
         */
        apci_error("Unable to request region.\n");
        goto out_alloc_driver;
    }

    return ddata;

out_alloc_driver:
    kfree( ddata );
    return NULL;
}

/**
 * @desc Removes all of the drivers from this module
 *       before cleanup
 */
void 
apci_free_driver( struct pci_dev *pdev )
{ 
     struct apci_my_info *ddata = pci_get_drvdata( pdev );
     int count;

     apci_devel("Entering free driver.\n");

     apci_debug("releasing memory of %d , length=%d", ddata->plx_region.start, ddata->plx_region.length );
     release_region( ddata->plx_region.start, ddata->plx_region.length );

     for (count = 0; count < 6; count ++) {
          if (ddata->regions[count].start == 0) {
               continue; /* invalid region */
          }
          if (ddata->regions[count].flags & IORESOURCE_IO) {
               release_region(ddata->regions[count].start, ddata->regions[count].length);
          } else {
               iounmap(ddata->regions[count].mapped_address);
               release_mem_region(ddata->regions[count].start, ddata->regions[count].length);
          }
     }
        
     kfree(ddata );
     apci_debug("Completed freeing driver.\n");
}

static void apci_class_dev_unregister(struct apci_my_info *ddata )
{

     apci_devel("entering apci_class_dev_unregister.\n");
     if (ddata->dev == NULL)
          return;

     device_unregister( ddata->dev );
     dev_counter --;

     apci_devel("leaving apci_class_dev_unregister.\n");
}

static int __devinit
apci_class_dev_register( struct apci_my_info *ddata, int id )
{
    int ret;
    struct apci_lookup_table_entry *obj = &acpi_lookup_table[ APCI_LOOKUP_FN( (int)ddata->id->device ) ];
    apci_devel("entering apci_class_dev_register\n");
    
    /* ddata->dev = device_create(class_apci, &ddata->pci_dev->dev , apci_first_dev + id, NULL, "foo%d", id );    */
    /* ddata->id->device */
    /* sprintf(buf,"apci/%s_%d", obj->name, obj->counter ++ ); */

    ddata->dev = device_create(class_apci, &ddata->pci_dev->dev , apci_first_dev + id, NULL, "apci/%s_%d", obj->name, obj->counter ++ );

    if( IS_ERR( ddata->dev )) {
      apci_error("Error creating device");
      ret = PTR_ERR( ddata->dev );
      ddata->dev = NULL;
      return ret;
    }

    apci_devel("Leaving apci_class_dev_register\n");
    return 0;
}


void remove(struct pci_dev *pdev)
{
     struct apci_my_info *ddata = pci_get_drvdata( pdev );

     apci_devel("entering remove");

     apci_class_dev_unregister( ddata );

     cdev_del( &ddata->cdev );
     apci_free_driver( pdev );
     
     apci_devel("Leaving remove");
}



int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct apci_my_info *ddata;
    int ret;
    apci_devel("entering probe");
    
    if( pci_enable_device(pdev) ) {
      return -ENODEV;
    }

    ddata = (struct apci_my_info*) apci_alloc_driver( pdev, id  );
    if( ddata == NULL ) {
      return -ENOMEM;
    }

    ddata->nchannels = APCI_NCHANNELS;
    ddata->pci_dev   = pdev;
    ddata->id        = id;

    /* Spin lock init stuff */

    
    /* Request Irq */

    cdev_init( &ddata->cdev, &apci_fops );
    ddata->cdev.owner = THIS_MODULE;

    ret = cdev_add( &ddata->cdev, apci_first_dev + dev_counter, 1 );
    if ( ret ) { 
      apci_error("error registering Driver %d", dev_counter );
      goto exit_apci_alloc_driver;
    }
    
    pci_set_drvdata(pdev, ddata );
    ret = apci_class_dev_register( ddata , dev_counter );

    if ( ret )  
        goto exit_apci_cdev_del;

    dev_counter ++;

    apci_debug("Added driver %d", dev_counter - 1);
    return 0;
    
exit_apci_cdev_del:
    cdev_del( &ddata->cdev );
exit_apci_alloc_driver:

    apci_free_driver( pdev );
        
    return ret;
}

 
int __init 
apci_init(void)
{
	void *ptr_err;
        int result;
        int ret;
        dev_t dev = MKDEV(0,0);
        apci_debug("performing init duties");
        INIT_LIST_HEAD( &head.driver_list );

     
#if 1
        ret = alloc_chrdev_region( &apci_first_dev, 0, MAX_APCI_CARDS , APCI );
        if( ret ) {
          apci_error("Unable to allocate device numbers");
          return ret;
        }

#endif
        
        /* Create the sysfs entry for this */
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

        apci_error("Unregistering chrdev_region.\n");

        unregister_chrdev_region(MKDEV(major_num,0),1 );

	class_destroy(class_apci);
	return PTR_ERR(ptr_err);
}

static void __exit apci_exit(void)
{
    apci_debug("performing exit duties");
    pci_unregister_driver(&pci_driver);
    cdev_del(&apci_cdev);
    unregister_chrdev_region(MKDEV(major_num,0),1 );
    class_destroy(class_apci );
}

module_init( apci_init  );
module_exit( apci_exit  );

MODULE_AUTHOR("Jimi Damon <jdamon@accesio.com>");
MODULE_LICENSE("GPL");
