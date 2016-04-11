CC = gcc
RM = rm

IPC_SERVER_OBJS = ipc_server.o
IPC_CLIENT_OBJS = ipc_client.o

IPC_SERVER = ipc_server_demo
IPC_CLIENT = ipc_client_demo

all: $(IPC_SERVER) $(IPC_CLIENT)

$(IPC_SERVER): $(IPC_SERVER_OBJS)
	$(CC) $(IPC_SERVER_OBJS) -o $@

$(IPC_CLIENT): $(IPC_CLIENT_OBJS)
	$(CC) $(IPC_CLIENT_OBJS) -o $@

%.o: %.c
	$(CC) -c $<

clean:
	$(RM) *.o $(IPC_SERVER) $(IPC_CLIENT) ipctest.socket
