// ool_port_ipc_common.h
   
#ifndef _OOL_PORT_IPC_COMMON_H_
#define _OOL_PORT_IPC_COMMON_H_
   
...
#define SERVICE_NAME   "com.osxbook.ProcessorInfoServer"
...
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body; // start of kernel-processed data
    mach_msg_port_descriptor_t data; // end of kernel-processed data
} msg_format_response_t;
   
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body; // start of kernel-processed data
    mach_msg_port_descriptor_t data; // end of kernel-processed data
    mach_msg_trailer_t         trailer;
} msg_format_response_r_t;
   
#endif // _OOL_PORT_IPC_COMMON_H_
