// DummySysctl.c
   
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/sysctl.h>
   
static u_int32_t k_uint32 = 0;        // the contents of debug.osxbook.uint32
static u_int8_t k_string[16] = { 0 }; // the contents of debug.osxbook.string
   
// Construct a node (debug.osxbook) from which other sysctl objects can hang.
SYSCTL_NODE(_debug,       // our parent
            OID_AUTO,     // automatically assign us an object ID
            osxbook,      // our name
            CTLFLAG_RW,   // we will be creating children, therefore, read/write
            0,            // handler function (none needed)
            "demo sysctl hierarchy");
   
// Prototypes for read/write handling functions for our sysctl nodes.
static int sysctl_osxbook_uint32 SYSCTL_HANDLER_ARGS;
static int sysctl_osxbook_string SYSCTL_HANDLER_ARGS;
   
// We can directly use SYSCTL_INT(), in which case sysctl_handle_int()
// will be assigned as the handling function. We use SYSCTL_PROC() and
// specify our own handler sysctl_osxbook_uint32().
//
SYSCTL_PROC(
    _debug_osxbook,                 // our parent
    OID_AUTO,                       // automatically assign us an object ID
    uint32,                         // our name
    (CTLTYPE_INT |                  // type flag
     CTLFLAG_RW | CTLFLAG_ANYBODY), // access flags (read/write by anybody)
    &k_uint32,                      // location of our data
    0,                              // argument passed to our handler
    sysctl_osxbook_uint32,          // our handler function
    "IU",                           // our data type (unsigned integer)
    "32-bit unsigned integer"       // our description
);
   
// We can directly use SYSCTL_STRING(), in which case sysctl_handle_string()
// will be assigned as the handling function. We use SYSCTL_PROC() and
// specify our own handler sysctl_osxbook_string().
//
SYSCTL_PROC(
    _debug_osxbook,                // our parent
    OID_AUTO,                      // automatically assign us an object ID
    string,                        // our name
    (CTLTYPE_STRING | CTLFLAG_RW), // type and access flags (write only by root)
    &k_string,                     // location of our data
    16,                            // maximum allowable length of the string
    sysctl_osxbook_string,         // our handler function
    "A",                           // our data type (string)
    "16-byte string"               // our description
);
   
static int
sysctl_osxbook_uint32 SYSCTL_HANDLER_ARGS
{
    // Do some processing of our own, if necessary.
    return sysctl_handle_int(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
}
   
static int
sysctl_osxbook_string SYSCTL_HANDLER_ARGS
{
    // Do some processing of our own, if necessary.
    return sysctl_handle_string(oidp, oidp->oid_arg1, oidp->oid_arg2, req);
}
   
kern_return_t
DummySysctl_start(kmod_info_t *ki, void *d)
{
    // Register our sysctl entries.
    sysctl_register_oid(&sysctl__debug_osxbook);
    sysctl_register_oid(&sysctl__debug_osxbook_uint32);
    sysctl_register_oid(&sysctl__debug_osxbook_string);
   
    return KERN_SUCCESS;
}
   
   
kern_return_t
DummySysctl_stop(kmod_info_t *ki, void *d)
{
    // Unregister our sysctl entries.
    sysctl_unregister_oid(&sysctl__debug_osxbook_string);
    sysctl_unregister_oid(&sysctl__debug_osxbook_uint32);
    sysctl_unregister_oid(&sysctl__debug_osxbook);
   
    return KERN_SUCCESS;
}
