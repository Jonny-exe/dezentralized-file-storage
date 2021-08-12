int searchFile(server_t *server, char *originIP, int IPSize, char *fileHash) {
  row_t hashtable[200];
  read_table("local_hash_table", hashtable, &lines);
  int length = sizeof(hashtable) / sizeof(row_t);
  int err, i;

  for (int i = 0; i < length; i++) {
    result = connectToPeer(originIP, fileHash, IPSize)
    if (result)
      // Found file owner
      break
  }
  return 0;
}


int connectToPeer(char *originIP, char *hash, int IPSize) {
  int type = 1, PORT = 8080, depth = 0, err;
  server_t server = {0};
  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("client: Failed to create socket endpoint\n");
    return err;
  }

  err = server_connect(&server, LOCALHOST, PORT);
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
  err = read(server.listen_fd, result, sizeof(int));
  if (err == -1) {
    perror("read");
    printf("client: Failed reading reuslt\n");
  }
  connection_close(server.listen_fd);
  return result;
} 
