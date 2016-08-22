// fd_receiver.c
   
#include "fd_common.h"
   
int receive_fd_using_sockfd(int *fd, int sockfd);
   
int
receive_fd_using_sockfd(int *fd, int sockfd)
{
    ssize_t                ret;
    u_char                 c;
    int                    errcond = 0;
    struct iovec           iovec[1];
    struct msghdr          msg;
    struct cmsghdr        *cmsghdrp;
    cmsghdr_msg_control_t  cmsghdr_msg_control;
   
    iovec[0].iov_base = &c;
    iovec[0].iov_len = 1;
   
    msg.msg_name = (caddr_t)0;
    msg.msg_namelen = 0;
    msg.msg_iov = iovec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsghdr_msg_control.msg_control;
    msg.msg_controllen = sizeof(cmsghdr_msg_control.msg_control);
    msg.msg_flags = 0;
   
    if ((ret = recvmsg(sockfd, &msg, 0)) <= 0) {
        perror("recvmsg");
        return ret;
    }
   
    cmsghdrp = CMSG_FIRSTHDR(&msg);
   
    if (cmsghdrp == NULL) {
        *fd = -1;
        return ret;
    }
   
    if (cmsghdrp->cmsg_len != CMSG_LEN(sizeof(int)))
        errcond++;
   
    if (cmsghdrp->cmsg_level != SOL_SOCKET)
        errcond++;
   
    if (cmsghdrp->cmsg_type != SCM_RIGHTS)
        errcond++;
   
    if (errcond) {
        fprintf(stderr, "%d errors in received message\n", errcond);
        *fd = -1;
    } else
        *fd = *((int *)CMSG_DATA(cmsghdrp));
   
    return ret;
}
   
int
main(int argc, char **argv)
{
    char               buf[512];
    int                fd = -1, sockfd, len, ret;
    struct sockaddr_un server_unix_addr;
   
    bzero((char *)&server_unix_addr, sizeof(server_unix_addr));
    strcpy(server_unix_addr.sun_path, SERVER_NAME);
    server_unix_addr.sun_family = AF_LOCAL;
    len = strlen(SERVER_NAME) + 1;
    len += sizeof(server_unix_addr.sun_family);
   
    if ((sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
   
    if (connect(sockfd, (struct sockaddr *)&server_unix_addr, len) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }
   
    ret = receive_fd_using_sockfd(&fd, sockfd);
   
    if ((ret < 0) || (fd < 0)) {
        fprintf(stderr, "failed to receive file descriptor\n");
        close(sockfd);
        exit(1);
    }
   
    printf("received file descriptor (fd = %d)\n", fd);
    if ((ret = read(fd, buf, 512)) > 0)
        write(1, buf, ret);
   
    exit(0);
}
