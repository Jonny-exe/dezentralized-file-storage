int searchFileLocation(char *fileHash, char *location);
int connectToPeer(char *targetIP, char *hash, char *location);
int askForBytes(char *ip, char *hash, int *bytes);

int searchFileLocation(char *fileHash, char *location) {
  int length;
  row_t hashtable[200];
  read_table("local_hash_table", hashtable, &length);
  int i, result;


  for (i = 0; i < length; i++) {
    result = connectToPeer(hashtable[i].ip, fileHash, location);
    if (result) {
      return 1;
    }
  }
  return 0;
}


int connectToPeer(char *targetIP, char *hash, char *location) {
  int type = 0, depth = 0, err;
  printf("Target IP %s\n", targetIP);
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

  int code;
  err = read(server.listen_fd, &code, sizeof(int));
  if (err == -1) {
    perror("read");
    printf("client: Failed reading reuslt\n");
  }

  if (code) {
    err = read(server.listen_fd, location, 20);
    if (err == -1) {
      perror("read");
      printf("Error reading location\n");
    }
  }
  connection_close(server.listen_fd);
  return code;
} 

int askForBytes(char *ip, char *hash, int *bytes) {
  int type = 2, err, code;
  server_t server = {0};
  err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
  if (err == -1) {
    perror("socket");
    printf("client: Failed to create socket endpoint\n");
    return err;
  }

  err = server_connect(&server, ip, PORT);
  if (err == -1) {
    perror("connect");
    printf("Error connecting\n");
  }

  err = write(server.listen_fd, &type, sizeof(int));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting type\n");
  }

  err = write(server.listen_fd, hash, 41);
  if (err == -1) {
    perror("write");
    printf("client: Failed writting type\n");
  }
  printf("Hash: %s\n", hash);

  err = read(server.listen_fd, &code, sizeof(int));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting type\n");
  }
  
  if (code == 0)
    printf("Doesn't have file");

  err = read(server.listen_fd, bytes, 256);
  connection_close(server.listen_fd);
  return 0;
}
