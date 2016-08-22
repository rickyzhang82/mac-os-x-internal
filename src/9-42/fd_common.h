// fd_common.h
   
#ifndef _FD_COMMON_H_
#define _FD_COMMON_H_
   
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
   
#define SERVER_NAME "/tmp/.fdserver"
   
typedef union {
    struct cmsghdr cmsghdr;
    u_char         msg_control[CMSG_SPACE(sizeof(int))];
} cmsghdr_msg_control_t;
   
#endif // _FD_COMMON_H_
