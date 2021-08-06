#include "helpers.c"
#include "socket.c"
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#define MAX_FILENAME 80
#define TEMP_FILE "temp.tar"
#define OTHERS_FILES "tempdir/"
#define FILE_LIST "local_file_list"

int createFakeTree(char files[MAX_FILENAME][100], int size);
int handleFile(char *filename, server_t *server);
int main(int argc, char *argv[]) {
  int err;
  char dirname[100] = "./test";
  if (fork() == 0) {
    // Handle all the initial files and the TCP server
    int PORT = atoi(argv[2]);
    server_t server = {0};

    err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
    if (err == -1) {
      perror("socket");
      printf("client: Failed to create socket endpoint\n");
      return err;
    }

    printf("Argv: %s\n", argv[1]);
    if (strcmp(argv[1], "send") == 0) {
      char files[100][MAX_FILENAME];
      int index, i;

      readFolderFiles(dirname, files, &index);
      if (index != 0) {
        err = server_connect(&server, PORT);
        if (err) {
          perror("connect");
          printf("client: Failed to connect to socket\n");
          return err;
        } else {
          printf("client: Connected to socket\n");
        }
      }

      for (i = 0; i < index; i++) {
        printf("File: %s\n", files[i]);
        handleFile(files[i], &server);
      }
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

    while (1) {
      int conn_fd;
      err = server_accept(&server, &conn_fd);
      if (err == -1) {
        perror("accept");
        printf("server: Failed accepting connection\n");
        return err;
      }

      int times, i = 0;
      err = read(conn_fd, &times, sizeof(times));
      if (err == -1) {
        perror("read");
        printf("client: Failed reading message\n");
        return err;
      }

      err = write(conn_fd, &i, sizeof(int));
      if (err == -1) {
        perror("write");
        printf("client: Failed writting message\n");
        return err;
      }

      int zero = 0;
      err = mkdir("tempdir/", 0777);
      if (err == -1) {
        printf("Error creating temp dir\n");
        perror("mkdir");
      }

      printf("Times: %d\n", times);
      for (i = 0; i < times; i++) {
        char hash[80];
        int bytes[64];
        err = read(conn_fd, hash, 80);
        if (err == -1) {
          perror("read");
          printf("server: Failed reading message\n");
          return err;
        }
        printf("Hash: %s, %d\n", hash, err);

        err = write(conn_fd, &zero, sizeof(int));
        if (err == -1) {
          perror("write");
          printf("server: Failed writting message\n");
          return err;
        }

        err = read(conn_fd, bytes, 256);
        if (err == -1) {
          perror("read");
          printf("server: Failed reading message\n");
          return err;
        }

        err = write(conn_fd, &zero, sizeof(int));
        if (err == -1) {
          perror("write");
          printf("server: Failed writting message\n");
          return err;
        }

        int idx;
        char filename[60] = "tempdir/";
        strcat(filename, hash);
        FILE *file = fopen(filename, "wb");
        for (idx = 0; idx < 64; idx++)
          fputc(bytes[idx], file);
        fclose(file);
      }

      printf("Close");
      err = connection_close(conn_fd);
      if (err == -1) {
        perror("close");
        printf("server: Failed closing connection\n");
        return err;
      }
    }
  } else {
    // Handle the file listening
    listenFolder(dirname);
    exit(0);
  }
  return 0;
}

int createFakeTree(char files[MAX_FILENAME][100], int size) {
  int i;
  for (i = 0; i < size; i++) {
    (void)createFile(files[i]);
  }
  return 0;
}

int handleFile(char *filename, server_t *server) {
  int err, i;
  char command[100];
  //sprintf(command, "mv '%s' temp",  filename);
  //err = system(command);
  err = rename(filename, "temp");
  if (err == -1)
    printf("Error moving file\n");

  err = compressFile("temp");
  if (err == -1)
    printf("Error compressing file\n");
  strcat(filename, ".gz");
  printf("Move: %s\n", filename);
  sprintf(command, "mv temp.gz '%s'",  filename);
  err = system(command);

  FILE *file = fopen(filename, "rb");

  int size = getFileSize(file);
  int times = ceil((double)size / (double)64);
  int bytes[times][256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes, times);

  fclose(file);

  unsigned char hashes[times][80];
  for (i = 0; i < times; i++) {
    hashFile(bytes[i], 64, hashes[i]);
  }
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");
  strcat(filename, ".cpt");
  char file2Remove[100];
  strcpy(file2Remove, filename);

  createFile(filename);
  strcat(filename, ".f");
  file = fopen(filename, "w");
  for (i = 0; i < times; i++) {
    fprintf(file, "%s\n", hashes[i]);
  }
  fclose(file);

  int code;
  err = write(server->listen_fd, &times, sizeof(times));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting message\n");
  }

  err = read(server->listen_fd, &code, sizeof(code));
  if (err == -1 && code == 0) {
    perror("read");
    printf("client: Failed reading message\n");
    return err;
    }
  for (i = 0; i < times; i++) {
    // Steps: 
    // 1. Send times
    // 2. Send hash
    // 3. Send bytes
    // 4. Repeat 2 and 3 for times

    printf("Hash: %s\n", hashes[i]);
    err = write(server->listen_fd, hashes[i], sizeof(hashes[i]));
    if (err == -1) {
      perror("write");
      printf("client: Failed writting message\n");
    }

    err = read(server->listen_fd, &code, sizeof(code));
    if (err == -1) {
      perror("read");
      printf("client: Failed reading message\n");
      return err;
    }

    err = write(server->listen_fd, bytes[i], sizeof(bytes[i]));
    if (err == -1) {
      perror("write");
      printf("client: Failed writting message\n");
      return err;
    }

    err = read(server->listen_fd, &code, sizeof(code));
    if (err == -1 && code == 0) {
      perror("read");
      printf("client: Failed reading message\n");
      return err;
    }
  }
  remove(file2Remove);
  return 0;
}

