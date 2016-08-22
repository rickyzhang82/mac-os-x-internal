// misc_types.h
   
#ifndef _MISC_TYPES_H_
#define _MISC_TYPES_H_
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
   
// The server port will be registered under this name.
#define MIG_MISC_SERVICE "MIG-miscservice"
   
// Data representations
typedef char input_string_t[64];
typedef int  xput_number_t;
   
typedef struct {
    mach_msg_header_t head;
   
    // The following fields do not represent the actual layout of the request
    // and reply messages that MIG will use. However, a request or reply
    // message will not be larger in size than the sum of the sizes of these
    // fields. We need the size to put an upper bound on the size of an
    // incoming message in a mach_msg() call.
    NDR_record_t NDR;
    union {
        input_string_t string;
        xput_number_t  number;
    } data;
    kern_return_t      RetCode;
    mach_msg_trailer_t trailer;
} msg_misc_t;
   
xput_number_t misc_translate_int_to_xput_number_t(int);
int           misc_translate_xput_number_t_to_int(xput_number_t);
void          misc_remove_reference(xput_number_t);
kern_return_t string_length(mach_port_t, input_string_t, xput_number_t *);
kern_return_t factorial(mach_port_t, xput_number_t, xput_number_t *);
   
#endif // _MISC_TYPES_H_
