#ifndef APCI_COMMON_H
#define APCI_COMMON_H


#define APCI_PREFIX "apci: "
#define APCI "apci"

#define APCI_NCHANNELS 8 
#define MAX_APCI_CARDS 8
#define MAX_APCI_DEVICES (MAX_APCI_CARDS * APCI_NCHANNELS)


/**
 * Debugging , simplified
 */

#ifdef A_PCI_DEBUG
#define apci_debug(fmt,...) pr_debug(APCI_PREFIX fmt, ##__VA_ARGS__ )
#define apci_devel(fmt,...) pr_debug(APCI_PREFIX fmt, ##__VA_ARGS__ )
#else 
#define apci_debug(fmt,...) if( 0 ) { pr_debug(APCI_PREFIX fmt, ##__VA_ARGS__ ); }
#define apci_devel(fmt,...) if( 0 ) { pr_debug(APCI_PREFIX fmt, ##__VA_ARGS__ ); }
#endif
#define apci_error(fmt,...) pr_err( APCI_PREFIX fmt, ##__VA_ARGS__ )
#define apci_info(fmt, ...) pr_info( APCI_PREFIX fmt, ##__VA_ARGS__ )


#endif
