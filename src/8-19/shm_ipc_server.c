// shm_ipc_server.c
   
#include <stdio.h>
#include <stdlib.h>
#include "shm_ipc_common.h"
   
int
main(void)
{
    char                  *ptr;
    kern_return_t          kr;
    mach_vm_address_t      address = 0;
    memory_object_size_t   size = (memory_object_size_t)vm_page_size;
    mach_port_t            object_handle = MACH_PORT_NULL;
    msg_format_request_r_t recv_msg;
    msg_format_response_t  send_msg;
    mach_msg_header_t     *recv_hdr, *send_hdr;
    mach_port_t            server_port;
   
    kr = mach_vm_allocate(mach_task_self(), &address, size, VM_FLAGS_ANYWHERE);
    EXIT_ON_MACH_ERROR("vm_allocate", kr, KERN_SUCCESS);
   
    printf("memory allocated at %p\n", (void *)address);
   
    // Create a named entry corresponding to the given mapped portion of our
    // address space. We can then share this named entry with other tasks.
    kr = mach_make_memory_entry_64(
             (vm_map_t)mach_task_self(),              // target address map
             &size,                                   // so many bytes
             (memory_object_offset_t)address,         // at this address
             (vm_prot_t)(VM_PROT_READ|VM_PROT_WRITE), // with these permissions
             (mem_entry_name_port_t *)&object_handle, // outcoming object handle
             (mem_entry_name_port_t)NULL);            // parent handle
    // ideally we should vm_deallocate() before we exit
    EXIT_ON_MACH_ERROR("mach_make_memory_entry", kr, KERN_SUCCESS);
   
    // put some data into the shared memory
    ptr = (char *)address;
    strcpy(ptr, "Hello, Mach!");
   
    // become a Mach server
    kr = bootstrap_create_service(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_create_service", kr, BOOTSTRAP_SUCCESS);
   
    kr = bootstrap_check_in(bootstrap_port, SERVICE_NAME, &server_port);
    EXIT_ON_MACH_ERROR("bootstrap_check_in", kr, BOOTSTRAP_SUCCESS);
   
    for (;;) { // server loop
   
        // receive a message
        recv_hdr                  = &(recv_msg.header);
        recv_hdr->msgh_local_port = server_port;
        recv_hdr->msgh_size       = sizeof(recv_msg);
        kr = mach_msg(recv_hdr,              // message buffer
                      MACH_RCV_MSG,          // option indicating service
                      0,                     // send size
                      recv_hdr->msgh_size,   // size of header + body
                      server_port,           // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(recv)", kr, KERN_SUCCESS);
   
        // send named entry object handle as the reply
        send_hdr                   = &(send_msg.header);
        send_hdr->msgh_bits        = MACH_MSGH_BITS_LOCAL(recv_hdr->msgh_bits);
        send_hdr->msgh_bits       |= MACH_MSGH_BITS_COMPLEX;
        send_hdr->msgh_size        = sizeof(send_msg);
        send_hdr->msgh_local_port  = MACH_PORT_NULL;
        send_hdr->msgh_remote_port = recv_hdr->msgh_remote_port;
        send_hdr->msgh_id          = recv_hdr->msgh_id;
        send_msg.body.msgh_descriptor_count = 1;
        send_msg.data.name                  = object_handle;
        send_msg.data.disposition           = MACH_MSG_TYPE_COPY_SEND;
        send_msg.data.type                  = MACH_MSG_PORT_DESCRIPTOR;
        kr = mach_msg(send_hdr,              // message buffer
                      MACH_SEND_MSG,         // option indicating send
                      send_hdr->msgh_size,   // size of header + body
                      0,                     // receive limit
                      MACH_PORT_NULL,        // receive name
                      MACH_MSG_TIMEOUT_NONE, // no timeout, wait forever
                      MACH_PORT_NULL);       // no notification port
        EXIT_ON_MACH_ERROR("mach_msg(send)", kr, KERN_SUCCESS);
    }
   
    mach_port_deallocate(mach_task_self(), object_handle);
    mach_vm_deallocate(mach_task_self(), address, size);
   
    return kr;
}
