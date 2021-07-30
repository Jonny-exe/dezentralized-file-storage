#include <stdio.h>
#include "socket.c"

int main() {
  int err = 0;
  int PORT = 8080;
  server_t server = {0};

  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("client: Failed to create socket endpoint\n");
    return err;
  }

  err = server_connect(&server, PORT);
  if (err) {
    perror("connect");
    printf("client: Failed to connect to socket\n");
    return err;
  } else {
    printf("client: Connected to socket\n");
  }

  char message[20] = {0};
  char message1[20] = "Hello from client";

  err = server_write(&server, message1, sizeof(message));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting message\n");
  }

  err = server_read(&server, message, sizeof(message));
  if (err == -1) {
    perror("read");
    printf("client: Failed reading message\n");
    return err;
  }
  printf("client: Message: %s\n", message);

  return 0;
}
