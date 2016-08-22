// shm_ipc_client.c
   
#include <stdio.h>
#include <stdlib.h>
#include "shm_ipc_common.h"
   
int
main(int argc, char **argv)
{
    kern_return_t            kr;
    msg_format_request_t     send_msg;
    msg_format_response_r_t  recv_msg;
    mach_msg_header_t       *send_hdr, *recv_hdr;
    mach_port_t              client_port, server_port, object_handle;
    
    // find the server
    kr = bootstrap_look_up(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_look_up", kr, BOOTSTRAP_SUCCESS);
   
    // allocate a port for receiving the server's reply
    kr = mach_port_allocate(mach_task_self(),        // our task is acquiring
                            MACH_PORT_RIGHT_RECEIVE, // a new receive right
                            &client_port);           // with this name
    EXIT_ON_MACH_ERROR("mach_port_allocate", kr, KERN_SUCCESS);
   
    // prepare and send a request message to the server
    send_hdr                   = &(send_msg.header);
    send_hdr->msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, \
                                                MACH_MSG_TYPE_MAKE_SEND);
    send_hdr->msgh_size        = sizeof(send_msg);
    send_hdr->msgh_remote_port = server_port;
    send_hdr->msgh_local_port  = client_port;
    send_hdr->msgh_reserved    = 0;
    send_hdr->msgh_id          = SHM_MSG_ID;
    kr = mach_msg(send_hdr,              // message buffer
                  MACH_SEND_MSG,         // option indicating send
                  send_hdr->msgh_size,   // size of header + body
                  0,                     // receive limit
                  MACH_PORT_NULL,        // receive name
                  MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                  MACH_PORT_NULL);       // no notification port
    EXIT_ON_MACH_ERROR("mach_msg(send)", kr, MACH_MSG_SUCCESS);
   
    do {
        recv_hdr                   = &(recv_msg.header);
        recv_hdr->msgh_remote_port = server_port;
        recv_hdr->msgh_local_port  = client_port;
        recv_hdr->msgh_size        = sizeof(recv_msg);
        recv_msg.data.name         = 0;
        kr = mach_msg(recv_hdr,              // message buffer
                      MACH_RCV_MSG,          // option indicating receive
                      0,                     // send size
                      recv_hdr->msgh_size,   // size of header + body
                      client_port,           // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(rcv)", kr, MACH_MSG_SUCCESS);
   
        printf("recv_msg.data.name = %#08x\n", recv_msg.data.name);
        object_handle = recv_msg.data.name;
   
        { // map the specified memory object to a region of our address space
   
            mach_vm_size_t    size = vm_page_size;
            mach_vm_address_t address = 0;
   
            kr = mach_vm_map(
                     mach_task_self(),            // target address space (us)
                     (mach_vm_address_t *)&address, // map it and tell us where
                     (mach_vm_size_t)size,        // number of bytes to allocate
                     (mach_vm_offset_t)0,         // address mask for alignment
                     TRUE,                        // map it anywhere
                     (mem_entry_name_port_t)object_handle, // the memory object
                     (memory_object_offset_t)0,   // offset within memory object
                     FALSE,                       // don't copy -- directly map
                     VM_PROT_READ|VM_PROT_WRITE,  // current protection
                     VM_PROT_READ|VM_PROT_WRITE,  // maximum protection
                     VM_INHERIT_NONE);            // inheritance properties
            if (kr != KERN_SUCCESS)
                mach_error("vm_map", kr);
            else {
                // display the current contents of the memory
                printf("%s\n", (char *)address); 
                if (argc == 2) { // write specified string to the memory
                    printf("writing \"%s\" to shared memory\n", argv[1]); 
                    strncpy((char *)address, argv[1], (size_t)size);
                    ((char *)address)[size - 1] = '\0';
                }
                mach_vm_deallocate(mach_task_self(), address, size);
            }
        }
    } while (recv_hdr->msgh_id != SHM_MSG_ID);
   
    exit(0);
}
