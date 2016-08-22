// shm_ipc_common.h
   
#ifndef _SHM_IPC_COMMON_H_
#define _SHM_IPC_COMMON_H_
   
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <servers/bootstrap.h>
   
#define SERVICE_NAME "com.osxbook.SHMServer"
#define SHM_MSG_ID   400
   
#define EXIT_ON_MACH_ERROR(msg, retval, success_retval) \
    if (kr != success_retval) { mach_error(msg ":" , kr); exit((retval)); }
   
// send-side version of the request message (as seen by the client)
typedef struct {
    mach_msg_header_t header;
} msg_format_request_t;
   
// receive-side version of the request message (as seen by the server)
typedef struct {
    mach_msg_header_t  header;
    mach_msg_trailer_t trailer;
} msg_format_request_r_t;
   
// send-side version of the response message (as seen by the server)
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body;   // start of kernel processed data
    mach_msg_port_descriptor_t data;   // end of kernel processed data
} msg_format_response_t;
   
// receive-side version of the response message (as seen by the client)
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body;   // start of kernel processed data
    mach_msg_port_descriptor_t data;   // end of kernel processed data
    mach_msg_trailer_t         trailer;
} msg_format_response_r_t;
   
#endif // _SHM_IPC_COMMON_H_
