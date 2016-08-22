// simple_ipc_client.c
   
#include <stdio.h>
#include <stdlib.h>
#include "simple_ipc_common.h"
   
int
main(int argc, char **argv)
{
    kern_return_t      kr;
    msg_format_recv_t  recv_msg;
    msg_format_send_t  send_msg;
    mach_msg_header_t *recv_hdr, *send_hdr;
    mach_port_t        client_port, server_port;
    
    kr = bootstrap_look_up(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_look_up", kr, BOOTSTRAP_SUCCESS);
   
    kr = mach_port_allocate(mach_task_self(),        // our task is acquiring
                            MACH_PORT_RIGHT_RECEIVE, // a new receive right
                            &client_port);           // with this name
    EXIT_ON_MACH_ERROR("mach_port_allocate", kr, KERN_SUCCESS);
   
    printf("client_port = %d, server_port = %d\n", client_port, server_port);
   
    // prepare request
    send_hdr                   = &(send_msg.header);
    send_hdr->msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, \
                                                MACH_MSG_TYPE_MAKE_SEND);
    send_hdr->msgh_size        = sizeof(send_msg);
    send_hdr->msgh_remote_port = server_port;
    send_hdr->msgh_local_port  = client_port;
    send_hdr->msgh_reserved    = 0;
    send_hdr->msgh_id          = DEFAULT_MSG_ID;
    send_msg.data              = 0;
   
    if (argc == 2)
        send_msg.data = atoi(argv[1]);
    if ((send_msg.data < 1) || (send_msg.data > 20))
        send_msg.data = 1; // some sane default value
   
    // send request
    kr = mach_msg(send_hdr,              // message buffer
                  MACH_SEND_MSG,         // option indicating send
                  send_hdr->msgh_size,   // size of header + body
                  0,                     // receive limit
                  MACH_PORT_NULL,        // receive name
                  MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                  MACH_PORT_NULL);       // no notification port
    EXIT_ON_MACH_ERROR("mach_msg(send)", kr, MACH_MSG_SUCCESS);
   
    do { // receive reply
        recv_hdr                   = &(recv_msg.header);
        recv_hdr->msgh_remote_port = server_port;
        recv_hdr->msgh_local_port  = client_port;
        recv_hdr->msgh_size        = sizeof(recv_msg);
   
        kr = mach_msg(recv_hdr,              // message buffer
                      MACH_RCV_MSG,          // option indicating receive
                      0,                     // send size
                      recv_hdr->msgh_size,   // size of header + body
                      client_port,           // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(recv)", kr, MACH_MSG_SUCCESS);
   
        printf("%d\n", recv_msg.data);
   
    } while (recv_hdr->msgh_id != DEFAULT_MSG_ID);
   
    exit(0);
}
