// simple_ipc_server.c
   
#include <stdio.h>
#include <stdlib.h>
#include "simple_ipc_common.h"
   
int
factorial(int n)
{
    if (n < 1)
        return 1;
    else return n * factorial(n - 1);
}
   
int
main(int argc, char **argv)
{
    kern_return_t      kr;
    msg_format_recv_t  recv_msg;
    msg_format_send_t  send_msg;
    mach_msg_header_t *recv_hdr, *send_hdr;
    mach_port_t        server_port;
   
    kr = bootstrap_create_service(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_create_service", kr, BOOTSTRAP_SUCCESS);
   
    kr = bootstrap_check_in(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_check_in", kr, BOOTSTRAP_SUCCESS);
   
    printf("server_port = %d\n", server_port);
   
    for (;;) { // server loop
   
        // receive message
        recv_hdr                  = &(recv_msg.header);
        recv_hdr->msgh_local_port = server_port;
        recv_hdr->msgh_size       = sizeof(recv_msg);
        kr = mach_msg(recv_hdr,              // message buffer
                      MACH_RCV_MSG,          // option indicating receive
                      0,                     // send size
                      recv_hdr->msgh_size,   // size of header + body
                      server_port,           // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(recv)", kr, MACH_MSG_SUCCESS);
   
        printf("recv data = %d, id = %d, local_port = %d, remote_port = %d\n",
               recv_msg.data, recv_hdr->msgh_id,
               recv_hdr->msgh_local_port, recv_hdr->msgh_remote_port);
   
        // process message and prepare reply
        send_hdr                   = &(send_msg.header);
        send_hdr->msgh_bits        = MACH_MSGH_BITS_LOCAL(recv_hdr->msgh_bits);
        send_hdr->msgh_size        = sizeof(send_msg);
        send_hdr->msgh_local_port  = MACH_PORT_NULL;
        send_hdr->msgh_remote_port = recv_hdr->msgh_remote_port;
        send_hdr->msgh_id          = recv_hdr->msgh_id;
        send_msg.data              = factorial(recv_msg.data);
   
        // send message
        kr = mach_msg(send_hdr,              // message buffer
                      MACH_SEND_MSG,         // option indicating send
                      send_hdr->msgh_size,   // size of header + body
                      0,                     // receive limit
                      MACH_PORT_NULL,        // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(send)", kr, MACH_MSG_SUCCESS);
   
        printf("reply sent\n");
    }
   
    exit(0);
}
