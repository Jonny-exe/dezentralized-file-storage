#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

// This server partly from: https://ops.tips/blog/a-tcp-server-in-c/#the-overview

typedef struct server {
  int listen_fd;
} server_t;

int server_accept(server_t *server);
int server_listen(server_t *server);
int server_bind(server_t *server, int PORT);
int server_connect(server_t *server, int PORT);
int server_write(server_t *server, char *buff, int size);
int server_read(server_t *server, char *buff, int size);

int server_accept(server_t *server) {
  int err = 0;
  int conn_fd;
  socklen_t client_len;
  struct sockaddr_in *client_addr;

  client_len = sizeof(client_addr);

  err = (conn_fd = accept(server->listen_fd, (struct sockaddr *)&client_addr,
                          &client_len));
  server->listen_fd = conn_fd;

  /*err = close(conn_fd);*/

  /*if (err == -1) {*/
    /*perror("close");*/
    /*printf("failed to close connection\n");*/
    /*return err;*/
  /*}*/

  return err;
}

int server_listen(server_t *server) {
  int BACKLOG = 4;
  int err;
  err = listen(server->listen_fd, BACKLOG);
  if (err == -1) {
    perror("listen");
    printf("Failed to put socket in passive mode\n");
    return err;
  }
  return err;
}

int server_bind(server_t *server, int PORT) {
  int err;
  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  err = bind(server->listen_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr));
  return err;
}

int server_connect(server_t *server, int PORT) {
  int err;
  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);
  err = connect(server->listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  return err;
}

int server_write(server_t *server, char *buff, int size) {
  int err;
  err = write(server->listen_fd, buff, size);
  printf("Wrote '%s'\n", buff);
  return err;
}

int server_read(server_t *server, char *buff, int size) {
  int err;
  err = read(server->listen_fd, buff, size);
  return err;
}
