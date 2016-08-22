// port_set_ipc_server.c
...
   
int
main(int argc, char **argv)
{
    ...
    mach_port_t        server_portset, server1_port, server2_port;
   
    // allocate a port set
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
                            &server_portset);
   
    // first service
    kr = bootstrap_create_service(bootstrap_port, SERVICE1_NAME, &server1_port);
    ...
    kr = bootstrap_check_in(bootstrap_port, SERVICE1_NAME, &server1_port);
    ...
   
    // second service
    kr = bootstrap_create_service(bootstrap_port, SERVICE2_NAME, &server2_port);
    ...
    kr = bootstrap_check_in(bootstrap_port, SERVICE2_NAME, &server2_port);
    ...
   
    // move right to the port set
    kr = mach_port_move_member(mach_task_self(), server1_port, server_portset);
    ...
   
    // move right to the port set
    kr = mach_port_move_member(mach_task_self(), server2_port, server_portset);
    ...
   
    for (;;) {
   
        // receive message on the port set
        kr = mach_msg(recv_hdr, ..., server_portset, ...);
        ...
   
        // determine target service and process
        if (recv_hdr->msgh_local_port == server1_port) {
            // processing for the first service
        } else if (recv_hdr->msgh_local_port == server2_port) {
            // processing for the second service
        } else {
            // unexpected!
        }
   
        // send reply
    }
    ...
}
