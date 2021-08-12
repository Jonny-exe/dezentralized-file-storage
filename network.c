int searchFileLocation(char *originIP, int IPSize, char *fileHash, char *location) {
  row_t hashtable[200];
  read_table("local_hash_table", hashtable, &lines);
  int length = sizeof(hashtable) / sizeof(row_t);
  int err, i;

  for (int i = 0; i < length; i++) {
    result = connectToPeer(originIP, hashtable[i], fileHash, IPSize, location)
    if (result) {
      return 1;
    }
  }
  return 0;
}


int connectToPeer(char *originIP, char *targetIP, char *hash, int IPSize, char *location) {
  int type = 1, PORT = 8080, depth = 0, err;
  server_t server = {0};
  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("client: Failed to create socket endpoint\n");
    return err;
  }

  err = server_connect(&server, targetIP, PORT);
  if (err == -1) {
    perror("connect");
    printf("Error connecting\n");
  }

  err = write(server.listen_fd, &type, sizeof(int));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting type\n");
  }

  err = write(server.listen_fd, &depth, sizeof(int));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting depth\n");
  }

  err = write(server.listen_fd, hash, 41);
  if (err == -1) {
    perror("write");
    printf("client: Failed writting hash\n");
  }

  err = write(server.listen_fd, IPSize, sizeof(int));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting hash\n");
  }

  err = write(server.listen_fd, originIP, IPSize);
  if (err == -1) {
    perror("write");
    printf("client: Failed writting hash\n");
  }

  int result;
  err = read(server.listen_fd, &result, sizeof(int));
  if (err == -1) {
    perror("read");
    printf("client: Failed reading reuslt\n");
  }
  if (result) {
    err = read(server.listen_fd, location, 100);
    if (err == -1) {
      perror("read");
      printf("Error reading location\n");
    }
  }
  connection_close(server.listen_fd);
  return result;
} 
