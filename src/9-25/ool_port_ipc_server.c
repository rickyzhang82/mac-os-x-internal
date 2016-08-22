// ool_port_ipc_server.c
   
int
main(int argc, char **argv)
{
    ...
    host_priv_t             host_priv;
    ...
   
    // acquire send rights to the host privileged port in host_priv
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
        send_msg.data.name                  = host_priv;
        send_msg.data.disposition           = MACH_MSG_TYPE_COPY_SEND;
        send_msg.data.type                  = MACH_MSG_PORT_DESCRIPTOR;
   
        // send response
        ...
    }
   
    exit(0);
}
