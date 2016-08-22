// simple_ipc_common.h
   
#ifndef _SIMPLE_IPC_COMMON_H_
#define _SIMPLE_IPC_COMMON_H_
   
#include <mach/mach.h>
#include <servers/bootstrap.h>
   
#define SERVICE_NAME   "com.osxbook.FactorialServer"
#define DEFAULT_MSG_ID 400
   
#define EXIT_ON_MACH_ERROR(msg, retval, success_retval) \
    if (kr != success_retval) { mach_error(msg ":" , kr); exit((retval)); }
   
typedef struct {
    mach_msg_header_t header;
    int               data;
} msg_format_send_t;
   
typedef struct {
    mach_msg_header_t  header;
    int                data;
    mach_msg_trailer_t trailer;
} msg_format_recv_t;
   
#endif // _SIMPLE_IPC_COMMON_H_
