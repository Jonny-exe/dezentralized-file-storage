#include <stdio.h>
#include "socket.c"

int main() {
  int PORT = 8080;
  int err = 0;
  server_t server = {0};

  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("server: Failed to create socket endpoint\n");
    return err;
  }

  err = server_bind(&server, PORT);
  if (err == -1) {
    perror("bind");
    printf("server: Failed to bind socket to address\n");
    return err;
  } else {
    printf("server: Bind-ed\n");
  }

  err = server_listen(&server);
  if (err == -1) {
    printf("server: Failed to listen on address 0.0.0.0:%d\n", PORT);
    return err;
  } else {
    printf("server: Server listening\n");
  }

  while (1) { // Check if this is working with lsof -i:PORT
    err = server_accept(&server);

    if (err == -1) {
      perror("accept");
      printf("server: Failed accepting connection\n");
      return err;
    }

    printf("server: Client connected!\n");

    char message[20];
    err = server_read(&server, message, sizeof(message));
    if (err == -1) {
      perror("read");
      printf("server: Failed reading message\n");
      return err;
    }

    char message1[20] = "Hello from server";
    err = server_write(&server, message1, sizeof(message));
    if (err == -1) {
      perror("write");
      printf("server: Failed writing message\n");
      return err;
    }

    printf("server: Message: %s\n", message);
    return 0;
  }
}
