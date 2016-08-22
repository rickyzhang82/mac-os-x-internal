// fd_sender.c
   
#include "fd_common.h"
   
int setup_server(const char *name);
int send_fd_using_sockfd(int fd, int sockfd);
   
int
setup_server(const char *name)
{
    int sockfd, len;
    struct sockaddr_un server_unix_addr;
   
    if ((sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return sockfd;
    }
   
    unlink(name);
   
    bzero((char *)&server_unix_addr, sizeof(server_unix_addr));
    server_unix_addr.sun_family = AF_LOCAL;
    strcpy(server_unix_addr.sun_path, name);
    len = strlen(name) + 1;
    len += sizeof(server_unix_addr.sun_family);
   
    if (bind(sockfd, (struct sockaddr *)&server_unix_addr, len) < 0) {
        close(sockfd);
        return -1;
    }
   
    return sockfd;
}
   
int
send_fd_using_sockfd(int fd, int sockfd)
{
    ssize_t                ret;
    struct iovec           iovec[1];
    struct msghdr          msg;
    struct cmsghdr        *cmsghdrp;
    cmsghdr_msg_control_t  cmsghdr_msg_control;
   
    iovec[0].iov_base = "";
    iovec[0].iov_len = 1;
   
    msg.msg_name = (caddr_t)0; // address (optional)
    msg.msg_namelen = 0;       // size of address
    msg.msg_iov = iovec;       // scatter/gather array
    msg.msg_iovlen = 1;        // members in msg.msg_iov
    msg.msg_control = cmsghdr_msg_control.msg_control; // ancillary data
    // ancillary data buffer length
    msg.msg_controllen = sizeof(cmsghdr_msg_control.msg_control);
    msg.msg_flags = 0;          // flags on received message
   
    // CMSG_FIRSTHDR() returns a pointer to the first cmsghdr structure in
    // the ancillary data associated with the given msghdr structure
    cmsghdrp = CMSG_FIRSTHDR(&msg);
   
    cmsghdrp->cmsg_len = CMSG_LEN(sizeof(int)); // data byte count
    cmsghdrp->cmsg_level = SOL_SOCKET;          // originating protocol
    cmsghdrp->cmsg_type = SCM_RIGHTS;           // protocol-specified type
   
    // CMSG_DATA() returns a pointer to the data array associated with
    // the cmsghdr structure pointed to by cmsghdrp
    *((int *)CMSG_DATA(cmsghdrp)) = fd;
   
    if ((ret = sendmsg(sockfd, &msg, 0)) < 0) {
        perror("sendmsg");
        return ret;
    }
   
    return 0;
}
   
int
main(int argc, char **argv)
{
    int                fd, sockfd;
    int                csockfd;
    socklen_t          len;
    struct sockaddr_un client_unix_addr;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file path>\n", argv[0]);
        exit(1);
    }
   
    if ((sockfd = setup_server(SERVER_NAME)) < 0) {
        fprintf(stderr, "failed to set up server\n");
        exit(1);
    }
   
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        perror("open");
        close(sockfd);
        exit(1);
    }
   
    listen(sockfd, 0);
   
    for (;;) {
        len = sizeof(client_unix_addr);
        csockfd = accept(sockfd, (struct sockaddr *)&client_unix_addr, &len);
        if (csockfd < 0) {
            perror("accept");
            close(sockfd);
            exit(1);
        }
   
        if ((send_fd_using_sockfd(fd, csockfd) < 0))
            fprintf(stderr, "failed to send file descriptor (fd = %d)\n", fd);
        else
            fprintf(stderr, "file descriptor sent (fd = %d)\n", fd);
   
        close(sockfd);
        close(csockfd);
        break;
    }
   
    exit(0);
}
