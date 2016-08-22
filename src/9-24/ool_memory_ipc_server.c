// ool_memory_ipc_server.c
   
...
#include "ool_memory_ipc_common.h"
   
// string we will send as OOL memory
const char *string = "abcdefghijklmnopqrstuvwxyz";
   
int
main(int argc, char **argv)
{
    ...
    msg_format_request_r_t recv_msg;
    msg_format_response_t  send_msg;
    ...
    for (;;) { // server loop
   
        // receive request
        ...
   
        // prepare response
        send_hdr                   = &(send_msg.header);
        send_hdr->msgh_bits        = MACH_MSGH_BITS_LOCAL(recv_hdr->msgh_bits);
        send_hdr->msgh_bits       |= MACH_MSGH_BITS_COMPLEX;
        send_hdr->msgh_size        = sizeof(send_msg);
        send_hdr->msgh_local_port  = MACH_PORT_NULL;
        send_hdr->msgh_remote_port = recv_hdr->msgh_remote_port;
        send_hdr->msgh_id          = recv_hdr->msgh_id;
   
        send_msg.body.msgh_descriptor_count = 1;
        send_msg.data.address               = (void *)string;
        send_msg.data.size                  = strlen(string) + 1;
        send_msg.data.deallocate            = FALSE;
        send_msg.data.copy                  = MACH_MSG_VIRTUAL_COPY;
        send_msg.data.type                  = MACH_MSG_OOL_DESCRIPTOR;
        send_msg.count                      = send_msg.data.size;
   
        // send response
        ...
    }
   
    exit(0);
}
