#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "socket.c"

int main() {
  int PORT = 8080;
  int err = 0;
  server_t server = {0};

  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("Failed to create socket endpoint\n");
    return err;
  }

  err = server_bind(&server, PORT);
  if (err == -1) {
    perror("bind");
    printf("Failed to bind socket to address\n");
    return err;
  } else {
    printf("Bind-ed\n");
  }

  err = server_listen(&server);
  if (err == -1) {
    printf("Failed to listen on address 0.0.0.0:%d\n", PORT);
    return err;
  } else {
    printf("Server listening\n");
  }

  while (1) { // Check if this is working with lsof -i:PORT
    err = server_accept(&server);

    if (err == -1) {
      perror("accept");
      printf("Failed accepting connection\n");
      return err;
    }

    printf("Client connected!\n");

    char message[20];
    err = server_read(&server, message, sizeof(message));
    if (err == -1) {
      perror("read");
      printf("Failed reading message\n");
      return err;
    }

    char message1[20] = "Hello from server";
    err = server_write(&server, message1, sizeof(message));
    if (err == -1) {
      perror("write");
      printf("Failed writing message\n");
      return err;
    }

    printf("Message: %s\n", message);
    return 0;
  }
}
