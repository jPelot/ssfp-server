#ifndef socket_h
#define socket_h

int start_server();
char* server_listen(int sockfd);
void server_send(char *str);
void server_close(int socket_listen);

int start_client(char *ip, char *port);
void client_send(char* message);
void client_close(int socket);
char *client_read();
char *read_fd(int fd);

#endif // socket_h
