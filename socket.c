#include <openssl/evp.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <unistd.h> // read(), write(), close()

//#include <openssl/applink.c>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "socket.h"

#define MAX 800
#define PORT 8080

SSL_CTX *ctx;

SSL *server_ssl;
int server_client;

SSL *client_ssl;

void InitializeSSL()
{
  SSL_load_error_strings();
  SSL_library_init();
  OpenSSL_add_all_algorithms();
}

void DestroySSL()
{
  ERR_free_strings();
  EVP_cleanup();
}

void ShutdownSSL(SSL *cSSL)
{
  SSL_shutdown(cSSL);
  SSL_free(cSSL);
}

int
start_server()
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    fprintf(stderr, "SSL_CTX_new() failed.\n");
    return 1;
  }

  if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM)
      || !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
    fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
    ERR_print_errors_fp(stderr);
    return 1;
  }

  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bind_address;
  getaddrinfo(0, "8080", &hints, &bind_address);

  printf("Creating socket...\n");
  int socket_listen;
  socket_listen = socket(bind_address->ai_family,
                         bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if(socket_listen < 0) {
    fprintf(stderr, "socket() failed. (%d)\n", errno);
    return 1;
  }

  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", errno);
    return 1;
  }

  return socket_listen;
}

int
start_client(char* hostname, char *port)
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    fprintf(stderr, "SSL_CTX_new() failed. \n");
    return 1;
  }

  //printf("Configuring remote address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *peer_address;
  if (getaddrinfo(hostname, port, &hints, &peer_address)) {
    fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
    return 1;
  }

  //printf("Remote address is: ");
  char address_buffer[100];
  char service_buffer[100];
  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
              address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
          NI_NUMERICHOST);
  //printf("%s %s\n", address_buffer, service_buffer);

  //printf("Creating socket...\n");
  int server;
  server = socket(peer_address->ai_family,
                  peer_address->ai_socktype,
                peer_address->ai_protocol);
  if(socket < 0) {
    fprintf(stderr, "socket() failed. (%d)\n", errno);
    return 1;
  }

  //printf("Connecting...\n");
  if (connect(server,
              peer_address->ai_addr,
            peer_address->ai_addrlen)) {
    fprintf(stderr, "connect() failed. (%d)\n", errno);
    return 1;
  }
  freeaddrinfo(peer_address);
  //printf("Connected.\n\n");

  SSL *ssl = SSL_new(ctx);
  if (!ctx) {
    fprintf(stderr, "SSL_new() failed.\n");
    return 1;
  }

  if (!SSL_set_tlsext_host_name(ssl, hostname)) {
    fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
    ERR_print_errors_fp(stderr);
    return 1;
  }

  SSL_set_fd(ssl, server);
  if (SSL_connect(ssl) == -1) {
    fprintf(stderr, "SSL_connect() failed.\n");
    ERR_print_errors_fp(stderr);
    return 1;
  }

  //printf("SSL/TLS using %s\n", SSL_get_cipher(ssl));

  X509 *cert = SSL_get_peer_certificate(ssl);

  /* TODO: CERTIFICATE VALIDATION */

  X509_free(cert);

  client_ssl = ssl;
  
  return server;
}

char*
server_listen(int socket_listen)
{
  
  printf("Listening...\n");
  if(listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", errno);
    return NULL;
  }

  printf("Waiting for connection...\n");
  struct sockaddr_storage client_address;
  socklen_t client_len = sizeof(client_address);
  int socket_client = accept(socket_listen,
                             (struct sockaddr*) &client_address,
                             &client_len);
  if (socket_client < 0) {
    fprintf(stderr, "accept() failed. (%d)\n", errno);
    return NULL;
  }

  printf("Client is connected.. ");
  char address_buffer[100];
  getnameinfo((struct sockaddr*)&client_address,
              client_len,
              address_buffer,
              sizeof(address_buffer),
              0, 0, NI_NUMERICHOST);
  printf("%s\n", address_buffer);

  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    fprintf(stderr, "SSL_new() failed. \n");
    return NULL;
  }

  SSL_set_fd(ssl, socket_client);
  if (SSL_accept(ssl) <= 0) {
    fprintf(stderr, "SSL_accept() failed.\n");
    ERR_print_errors_fp(stderr);
    SSL_shutdown(ssl);
    close(socket_client);
    SSL_free(ssl);
    return NULL;
  }

  printf("SSL connection using %s\n", SSL_get_cipher(ssl));

  printf("Reading request...\n");

  char request[1024];
  int bytes_received = SSL_read(ssl, request, 1024);
  printf("Received %d bytes.\n", bytes_received);
  request[bytes_received] = '\0';

  char *buffer = malloc(bytes_received + 1);
  memcpy(buffer, request, bytes_received+1);

  server_ssl = ssl;
  server_client = socket_client;
  
  return buffer;

}

void
server_send(char *str) {
  int bytes_sent =SSL_write(server_ssl, str, strlen(str));
  printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(str));

  printf("Closing connection...\n");
  SSL_shutdown(server_ssl);
  close(server_client);
  SSL_free(server_ssl);
}

void server_close(int socket_listen) {
  close(socket_listen);
  SSL_CTX_free(ctx);
}

void 
client_close(int socket)
{
  //printf("Closing socket...");
  SSL_shutdown(client_ssl);
  close(socket);
  SSL_free(client_ssl);
  SSL_CTX_free(ctx);
}

void
client_send(char* string) {
  SSL_write(client_ssl, string, strlen(string));
}

char*
client_read() {
  char *buffer = malloc(2048);
  char *buffptr = buffer;
  int bytes_read;
  int total_bytes = 0;
  while((bytes_read = SSL_read(client_ssl, buffptr, 2048-total_bytes)) > 0) {
    total_bytes += bytes_read;
    buffptr += bytes_read;
  }
  buffer[total_bytes] = '\0';
  return buffer;
}

char*
read_fd(int fd)
{
  char *buff = malloc(MAX);
  char *buffptr = buff;
  int bytes_read;
  int total_bytes = 0;
  while((bytes_read = read(fd, buffptr, MAX)) > 0) {
    total_bytes += bytes_read;
    buffptr += bytes_read;
  }
  buff[total_bytes] = '\0';
  //printf("From client:\n %s", buff);
  return buff;
}
