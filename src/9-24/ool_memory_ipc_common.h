// ool_memory_ipc_common.h
   
#ifndef _OOL_MEMORY_IPC_COMMON_H_
#define _OOL_MEMORY_IPC_COMMON_H_
   
...
#define SERVICE_NAME   "com.osxbook.OOLStringServer"
...
   
typedef struct {
    mach_msg_header_t header;
} msg_format_request_t;
   
typedef struct {
    mach_msg_header_t  header;
    mach_msg_trailer_t trailer;
} msg_format_request_r_t;
   
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body; // start of kernel-processed data
    mach_msg_ool_descriptor_t  data; // end of kernel-processed data
    mach_msg_type_number_t     count;
} msg_format_response_t;
   
typedef struct {
    mach_msg_header_t          header;
    mach_msg_body_t            body; // start of kernel-processed data
    mach_msg_ool_descriptor_t  data; // end of kernel-processed data
    mach_msg_type_number_t     count;
    mach_msg_trailer_t         trailer;
} msg_format_response_r_t;
   
#endif // _OOL_MEMORY_IPC_COMMON_H_
