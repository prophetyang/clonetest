#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
static struct sockaddr_un client = {0};
static const char *ipc_socket_file = "ipctest.socket";
int main(int argc, char **argv) {
    int rc=0, addrlen=0;
    int client_fd = -1;
    char *send_str = "I am IPC client";
    char recv_buf[128];
    int recv_len = 0;
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("%s: create socket failed: %s(%d)", 
            __FUNCTION__, strerror(errno), errno);
        return -1;
    }
    client.sun_family = AF_UNIX;
    strcpy(client.sun_path, ipc_socket_file);
    addrlen = sizeof(client.sun_family) + strlen(client.sun_path);
    if (connect(client_fd, (struct sockaddr *) &client, addrlen)){
        printf("%s: connect socket failed: %s(%d)", 
            __FUNCTION__, strerror(errno), errno);
        close(client_fd);
        return -1;
    }
    if (send(client_fd, send_str, strlen(send_str), 0) < 0) {
        printf("%s: send() failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        return;
    }
    printf("CLIENT: sent \"%s\"\n", send_str);
    if ((recv_len = recv(client_fd, recv_buf, 128, 0)) < 0) {
        printf("%s: recv() failed: %s(%d)",
            __FUNCTION__, strerror(errno), errno);
        return;
    }
    printf("CLIENT: received \"%s\"\n", recv_buf);
    close(client_fd);
    return 0;
}
