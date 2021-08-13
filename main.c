#include "hashtable.c"
#include "helpers.c"
#include "socket.c"
#include "network.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#define MAX_FILENAME 80
#define TEMP_FILE "temp.tar"
#define OTHERS_FILES "tempdir/"
#define FILE_LIST "local_file_list"
#define HASH_TABLE "local_hash_table"
#define LOCALHOST "127.0.0.1"
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int createFakeTree(char files[MAX_FILENAME][100], int size);
int handleFile(char *filename);
int listenFolder(char *dirname);
int main(int argc, char *argv[]) {
  int err;
  char dirname[100] = "./test";
  if (fork() == 0) {
    // Handle all the initial files and the TCP server
    int PORT = atoi(argv[2]);

    if (strcmp(argv[1], "send") == 0) {
      char files[100][MAX_FILENAME];
      int index, i;

      readFolderFiles(dirname, files, &index);
      for (i = 0; i < index; i++) {
        printf("File: %s\n", files[i]);
        handleFile(files[i]);
      }
      return 0;
    }

    server_t server = {0};

    err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
    if (err == -1) {
      perror("socket");
      printf("client: Failed to create socket endpoint\n");
      return err;
    }

    err = server_bind(&server, PORT);
    if (err == -1) {
      perror("bind");
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
      puts("Waiting to accept");
      err = server_accept(&server, &conn_fd);
      if (err == -1) {
        perror("accept");
        printf("server: Failed accepting connection\n");
        return err;
      } else {
        printf("Accepted connection\n");
      }

      int zero = 0, type = 0, i;
      // Type: the type of connection.
      //
      // 0 = request to find a file.
      // 1 = request to store files on this pc
      // 2 = request to get a file
      //
      // Code:
      // 0 = don't have the file
      // 1 = have the file

      err = read(conn_fd, &type, sizeof(int));
      if (err == -1) {
        perror("read");
        printf("client: Failed reading message\n");
        return err;
      }
      if (type == 1) {
        int times;
        err = read(conn_fd, &times, sizeof(times));
        if (err == -1) {
          perror("read");
          printf("client: Failed reading message\n");
          return err;
        }

        err = mkdir("tempdir/", 0777);
        if (err == -1) {
          perror("mkdir");
          printf("Error creating temp dir\n");
          err = 0;
        }
        printf("Times: %d\n", times);

        for (i = 0; i < times; i++) {
          char hash[80];
          int bytes[64];
          err = read(conn_fd, hash, 41);
          if (err == -1) {
            perror("read");
            printf("server: Failed reading message\n");
            return err;
          }

          err = read(conn_fd, bytes, 256);
          if (err == -1) {
            perror("read");
            printf("server: Failed reading message\n");
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

        printf("Close\n");
        err = connection_close(conn_fd);
        if (err == -1) {
          perror("close");
          printf("server: Failed closing connection\n");
          return err;
        }
      } else if (type == 0) {
        int depth, code = 0;
        err = read(conn_fd, &depth, sizeof(int));
        if (err == -1) {
          perror("read");
          printf("client: Failed reading depth\n");
          return err;
        }

        char hash[40];
        err = read(conn_fd, &hash, 41);
        if (err == -1) {
          perror("read");
          printf("client: Failed reading message\n");
          return err;
        }

        char filename[60] = OTHERS_FILES;
        strcat(filename, hash);
        if (access(filename, F_OK) != 0) {
          // Don't have the file
          printf("Don't have the file, %s\n", filename);
          if (depth == 10) {
            // Return error and exit
            err = write(conn_fd, &code, sizeof(int));
            if (err == -1) {
              perror("write");
              printf("Error writing result\n");
            }
            break;
          }

          char location[20];
          code = searchFileLocation(hash, location);
          err = write(conn_fd, &code, sizeof(int));
          if (err == -1) {
            perror("write");
            printf("Error writing result\n");
          }

          if (code) {
            write(conn_fd, location, 100);
            if (err == -1) {
              perror("write");
              printf("Error writing location\n");
            }
          }
          continue;
        }
        code = 1; // Everything OK, I have the file
        err = write(conn_fd, &code, sizeof(int));
        if (err == -1) {
          perror("write");
          printf("client: Failed writting message\n");
          return err;
        }

        //TODO: somehow find IP addr
        char ip[20] = "127.0.0.1";
        err = write(conn_fd, ip, 20);
        if (err == -1) {
          perror("write");
          printf("client: Failed writting message\n");
          return err;
        }
        connection_close(conn_fd);
      } else if (type == 2) {
        int code = 1;
        char hash[41];
        err = read(conn_fd, hash, 41);
        if (err == -1) {
          perror("read");
          printf("Error reading hash\n");
        }

        char filename[60] = OTHERS_FILES;
        strcat(filename, hash);
        printf("filename: %s\n", filename);
        if (access(filename, F_OK) != 0) {
          code = 0;
          err = write(conn_fd, &code, sizeof(int));
          if (err == -1) {
            perror("write");
            printf("Error writing code\n");
          }
          continue;
        }
        printf("Got the file\n");
        write(conn_fd, &code, sizeof(int));
        int bytes[64];
        int times, size;
        FILE *file = fopen(filename, "rb");
        size = getFileSize(file);
        times = ceil((double)size / (double)64);
        splitFile2Bytes(file, size, bytes, times);

        err = write(conn_fd, bytes, 256);
        if (err == -1)
          printf("Error sending bytes\n");
        else
          printf("Sent bytes\n");

        connection_close(conn_fd);
      }
    }
  } else {
    // Handle the file listening
    if (strcmp(argv[1], "send") != 0) {
      strcpy(dirname, "test2");
    }
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

int handleFile(char *filename) {
  int err, i;
  char command[100];
  err = rename(filename, "temp");
  if (err == -1)
    printf("Error moving file\n");

  err = compressFile("temp");
  if (err == -1)
    printf("Error compressing file\n");
  strcat(filename, ".gz");
  sprintf(command, "mv temp.gz '%s'", filename);
  err = system(command);
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");
  strcat(filename, ".cpt");
  char file2Remove[100];
  strcpy(file2Remove, filename);

  FILE *file = fopen(file2Remove, "rb");
  int size = getFileSize(file);
  int times = ceil((double)size / (double)64);
  int bytes[times][256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes, times);

  unsigned char hashes[times][41];
  for (i = 0; i < times; i++) {
    hashFile(bytes[i], 64, hashes[i]);
    printf("%s\n", hashes[i]);
  }

  createFile(filename);
  strcat(filename, ".f");
  file = fopen(filename, "w");
  for (i = 0; i < times; i++)
    fprintf(file, "%s\n", hashes[i]);

  fclose(file);

  int code, type = 1;
  int PORT = 8080;
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
    printf("client: Failed writting message\n");
  }

  err = write(server.listen_fd, &times, sizeof(times));
  if (err == -1) {
    perror("write");
    printf("client: Failed writting message\n");
  }

  for (i = 0; i < times; i++) {
    // Steps:
    // 1. Send times
    // 2. Send hash
    // 3. Send bytes
    // 4. Repeat 2 and 3 for times

    err = write(server.listen_fd, hashes[i], 41);
    if (err == -1) {
      perror("write");
      printf("client: Failed writting message\n");
    }
    err = write(server.listen_fd, bytes[i], sizeof(bytes[i]));
    if (err == -1) {
      perror("write");
      printf("client: Failed writting message\n");
      return err;
    }
  }
  remove(file2Remove);
  connection_close(server.listen_fd);
  return 0;
}

int receiveFile(char *originalFilename) {
  int lines, err, i;
  row_t hashtable[200];
  read_table(HASH_TABLE, hashtable, &lines);

  int hashIdx, size;
  FILE *file = fopen(originalFilename, "r");
  size = getFileSize(file);
  char hashes[size / 41][41];
  hashesFromFile(file, hashes, &hashIdx);
  int bytes[hashIdx][64];
  
  for (i = 0; i < hashIdx; i++) {
    char location[20];
    int result = searchFileLocation(hashes[i], location);
    if (result != 1) {
      printf("No file found\n");
      return -1;
    }

    printf("Location: %s\n", location);
    printf("Hash: %s\n", hashes[i]);
    err = askForBytes(location, hashes[i], bytes[i]);
    if (err == -1) {
      printf("Error asking for Bytes");
      return -1;
    }
    printf("Got bytes\n");
  }

  char filename[strlen(originalFilename) - 1];
  int j;
  memcpy(filename, originalFilename, strlen(originalFilename) - 2);
  filename[strlen(originalFilename) - 2] = '\0';
  file = fopen(filename, "wb");
  for (i = 0; i < hashIdx; i++) {
    for (j = 0; j < 64; j++)
      fputc(bytes[i][j], file);
  }
  fclose(file);

  err = cryptFile("a random key", filename, "decrypt");
  if (err == -1)
    printf("Error decrypting file\n");

  char zipfilename[strlen(filename) - 3];
  memcpy(zipfilename, filename, strlen(filename) - 4);
  zipfilename[strlen(filename) - 4] = '\0';
  err = uncompressFile(zipfilename);
  if (err == -1) {
    printf("Error uncompressing file\n");
  }
  return 0;
}

int listenFolder(char *dirname) {
  int length, i;
  int fd;
  int wd;

  while (1) {
    char buffer[BUF_LEN];
    i = 0;
    fd = inotify_init();

    if (fd < 0) {
      perror("inotify_init");
    }
    wd = inotify_add_watch(
        fd, dirname, IN_MODIFY | IN_CREATE | IN_DELETE | IN_ACCESS | IN_OPEN);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0) {
      perror("read");
    }

    while (i < length) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];
      if (event->len) {
        char filename[80];
        sprintf(filename, "%s/%s", dirname,event->name);

        if (event->mask & IN_CREATE && 1 == 0) {
          printf("The file %s was created.\n", event->name);
          handleFile(filename);
        } else if (event->mask & IN_DELETE) {
          printf("The file %s was deleted.\n", event->name);
        } else if (event->mask & IN_MODIFY) {
          printf("The file %s was modified.\n", event->name);
        } else if (event->mask & IN_ACCESS || event->mask & IN_OPEN) {
          printf("The file %s was accessed.\n", event->name);
          if (event->name[strlen(event->name) - 1] == 'f' &&
              event->name[strlen(event->name) - 2] == '.') {
            receiveFile(filename);
          }
        } 
      }
      i += EVENT_SIZE + event->len;
    }

    (void)inotify_rm_watch(fd, wd);
    (void)close(fd);
  }

  return 0;
}
