#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
/* For IPC domain sockets */
static struct sockaddr_un server = {0};
static int ipc_server_fd = -1;
static int ipc_client_fd = -1;
static const char *ipc_socket_file = "ipctest.socket";
/* For epoll */
#define MAX_EPOLL_EVENTS    10
static int epoll_fd = -1;

static int add_fd_to_epoll(int epoll_fd, int fd){
    int rc=0;
    struct epoll_event event;
     event.events = EPOLLIN|EPOLLHUP;
    event.data.fd = fd;
    if ((rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event)) < 0) {
        printf( "%s: add fd=%d to epoll failed: %s(%d)",
            __FUNCTION__, fd, strerror(errno), errno);
        return rc;
    }
    return 0;
}

static int del_fd_from_epoll(int epoll_fd, int fd) {
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

static void handle_client(int fd) {
    char *send_str = "I am IPC server";
    char recv_buf[128];
    int recv_len = 0;
    if ((recv_len = recv(fd, recv_buf, 128, 0)) < 0) {
        printf( "%s: recv() failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        return;
    }
    printf("SERVER: received \"%s\"\n", recv_buf);
    if (send(fd, send_str, strlen(send_str), 0) < 0) {
        printf( "%s: send() failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        return;
    }
     printf("SERVER: sent \"%s\"\n", send_str);
}

int main(int argc, char **argv) {
    int rc=0, addrlen=0;
    int numfds=0, epoll_index=0;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    /* init epoll_fd */
    if ((epoll_fd = epoll_create(10)) < 0) {
        printf( "%s: epoll_create() failed: %s(%d)\n",
            __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    /* init UNIX domain socket */
    if ((ipc_server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf( "%s: create socket failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    /* Set to Unix domain socket type */
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, ipc_socket_file);
    addrlen = sizeof(server.sun_family) + strlen(server.sun_path);
    /* remove pre-existing ipc_socket_file */
    unlink(server.sun_path);
    /* bind server socket */
    if (bind(ipc_server_fd, (struct sockaddr *) &server, addrlen)) {
        printf( "%s: bind socket failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        close(ipc_server_fd);
        return -1;
    }
    /* listen server socket */
    if (listen(ipc_server_fd, 5)) {
        printf( "%s: listen socket failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        close(ipc_server_fd);
        return -1;
    }
    add_fd_to_epoll(epoll_fd, ipc_server_fd);
    while (1) {
        numfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        for (epoll_index=0; epoll_index < numfds; epoll_index++) {
            if (ipc_server_fd == events[epoll_index].data.fd) {
                /* new client */
                int client_addr_len=0;
                struct sockaddr_un client = {0};
                if ((ipc_client_fd = accept(ipc_server_fd,
                                (struct sockaddr *)&client,
                                (socklen_t *)&client_addr_len)) < 0)
                {
                    printf( "%s: accept socket failed: %s(%d)",
                        __FUNCTION__, strerror(errno), errno);
                    continue;
                }
                printf("%s: accept new ipc client fd=%d\n",
                    __FUNCTION__, ipc_client_fd);
                add_fd_to_epoll(epoll_fd, ipc_client_fd);
            }
            else if (ipc_client_fd == events[epoll_index].data.fd) {
                if ((events[epoll_index].events & EPOLLERR) ||
                    (events[epoll_index].events & EPOLLHUP))
                {
                    printf("%s: close ipc client fd=%d\n",
                        __FUNCTION__, ipc_client_fd);
                    del_fd_from_epoll(epoll_fd, ipc_client_fd);
                    close(ipc_client_fd);
                    ipc_client_fd = -1;
                } else {
                    handle_client(ipc_client_fd);
                }
            }
        }
    }
    /* exception or exit */
    del_fd_from_epoll(epoll_fd, ipc_client_fd);
    del_fd_from_epoll(epoll_fd, ipc_server_fd);
    close(ipc_client_fd);
    close(ipc_server_fd);
    return 0;
}
