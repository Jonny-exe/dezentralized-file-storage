int PORT = 8080;
int OTHERS_FILES = getenv("OTHERS_FILES");
int BLOCK_LENGTH = 1024 * 256 / sizeof(int);
int BLOCK_SIZE = 1024 * 256;
char CONFIG_FILE_HALF_PATH[100] = "/Documents/GitHub/share-files/config/config";
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
#define FILE_LIST "local_file_list"
#define HASH_TABLE "local_hash_table"
#define LOCALHOST "127.0.0.1"
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

// Running gdb: gcc -Wall -g main.c -lcrypto -lm -o main
// Using it:    gcc -Wall main.c -lcrypto -lm -o main

int createFakeTree(char files[MAX_FILENAME][100], int size);
int handleFile(char *filename);
int listenFolder(char *dirname);
int main(int argc, char *argv[]) {
  if (OTHERS_FILES == NULL) {
    perror("getenv");
    printf("Error getting OTHERS_FILES env var");
    return 0;
  }
  int err;
  char dirname[100] = getenv("LISTEN_DIR");
  if (dirname == NULL)
    strcpy(dirname, "./test");

  char hashes[1][41];
  char CONFIG_FILE[100] = getenv("HOME");
  strcat(CONFIG_FILE, CONFIG_FILE_HALF_PATH);
  printf("Config file path: %s\n", CONFIG_FILE);
  FILE *file = fopen(CONFIG_FILE, "r");
  int hashIdx = 0;
  unsigned char userHash[40];
  err = hashesFromFile(file, hashes, &hashIdx);
  if (err == -1) {
    err = 0;
    createFile(CONFIG_FILE);
    file = fopen("config/config", "w");
    if (file == NULL) {
      printf("Problems with config file\n");
      //exit(0);
    }
    randomHash(userHash);
    fprintf(file, "%s", userHash);
    fclose(file);
  } else {
    fclose(file);
    memcpy(userHash, hashes[0], 37);
  }
  printf("User Hash: %s\n", userHash);

  if (fork() == 0) {
    // Handle all the initial files and the TCP server
    PORT = atoi(argv[2]);

    //Only for testing
    //if (strcmp(argv[1], "send") == 0) {
    char files[100][MAX_FILENAME];
    int index, i;

    readFolderFiles(dirname, files, &index);
    for (i = 0; i < index; i++) {
      printf("File: %s\n", files[i]);
      handleFile(files[i]);
    }
    return 0;
    //}

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

      int type = 0, i;
      // Type: the type of connection.
      //
      // 0 = request to find a file.
      // 1 = request to store files on this pc
      // 2 = request to get a file
      // 3 = request to remove a file
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
          int bytes[BLOCK_LENGTH];
          err = read(conn_fd, hash, 41);
          if (err == -1) {
            perror("read");
            printf("server: Failed reading message\n");
            return err;
          }

          err = read(conn_fd, bytes, BLOCK_SIZE);
          if (err == -1) {
            perror("read");
            printf("server: Failed reading message\n");
            return err;
          }

          int idx;
          char filename[60] = "tempdir/";
          strcat(filename, hash);
          FILE *file = fopen(filename, "wb");
          for (idx = 0; idx < BLOCK_LENGTH; idx++) {
            printf("%d\n", idx);
            fputc(bytes[idx], file);
          }
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

        // TODO: somehow find IP addr
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
        int bytes[BLOCK_LENGTH];
        int times, size;
        FILE *file = fopen(filename, "rb");
        size = getFileSize(file);
        times = ceil((double)size / (double)BLOCK_LENGTH);
        splitFile2Bytes(file, size, bytes, times);

        err = write(conn_fd, bytes, BLOCK_SIZE);
        if (err == -1)
          printf("Error sending bytes\n");
        else
          printf("Sent bytes\n");

        connection_close(conn_fd);
      } else if (type == 3) {
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
        err = write(conn_fd, &code, sizeof(int));
        if (err == -1) {
          perror("write");
          printf("Error writing code\n");
        }
        err = remove(filename);
        if (err == -1) {
          perror("remove");
          printf("Error removing file\n");
        }
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

int handleFile(char *originalFilename) {
  int err, i;
  char command[100];

  err = rename(originalFilename, "tmp/temp");
  if (err == -1)
    printf("Error moving file\n");

  char filename[20] = "tmp/temp";

  err = compressFile(filename);
  if (err == -1)
    printf("Error compressing file\n");
  strcat(filename, ".gz");
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");
  strcat(filename, ".cpt");
  char file2Remove[100];
  strcpy(file2Remove, filename);
  printf("Filename: %s\n", filename);

  FILE *file = fopen(file2Remove, "rb");
  int size = getFileSize(file);
  int times = ceil((double)size / (double)BLOCK_LENGTH);
  int bytes[times][BLOCK_LENGTH];
  splitFile2Bytes(file, size, bytes, times);
  printf("Times: %d\n", times);


  unsigned char hashes[times][41];
  for (i = 0; i < times; i++) {
    hashFile(bytes[i], BLOCK_LENGTH, hashes[i]);
    printf("%s\n", hashes[i]);
  }

  char finalFilename[strlen(originalFilename) + 9];
  strcpy(finalFilename, originalFilename);
  strcat(finalFilename, ".gz.cpt.f");
  printf("Final filename: %s\n", finalFilename);
  createFile(finalFilename);
  file = fopen(finalFilename, "w");
  for (i = 0; i < times; i++)
    fprintf(file, "%s\n", hashes[i]);

  fclose(file); 
  int type = 1;
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
    printf("Before send\n");
    err = write(server.listen_fd, bytes[i], BLOCK_SIZE);
    if (err == -1) {
      perror("write");
      printf("client: Failed writting message\n");
      return err;
    }
    printf("After send\n");
  }
  //remove(file2Remove);
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
  int bytes[hashIdx][BLOCK_LENGTH];

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

  char filename[20] = "tmp/temp.gz.cpt";
  int j;
  file = fopen(filename, "wb");
  for (i = 0; i < hashIdx; i++) {
    for (j = 0; j < BLOCK_LENGTH; j++)
      fputc(bytes[i][j], file);
  }
  fclose(file);

  printf("Filename 20: %s\n", filename);
  err = cryptFile("a random key", filename, "decrypt");
  if (err == -1)
    printf("Error decrypting file\n");

  char zipFilename[strlen(filename) - 3];
  memcpy(zipFilename, filename, strlen(filename) - 4);
  zipFilename[strlen(filename) - 4] = '\0';

  err = uncompressFile(zipFilename);
  if (err == -1) {
    printf("Error uncompressing file\n");
  }

  char unzipFilename[strlen(zipFilename) - 2];
  memcpy(unzipFilename, zipFilename, strlen(zipFilename) - 3);
  unzipFilename[strlen(zipFilename) - 3] = '\0';

  char finalFilename[strlen(originalFilename) - 8];
  memcpy(finalFilename, originalFilename, strlen(originalFilename) - 9);
  finalFilename[strlen(finalFilename)] = '\0';
  err = rename(unzipFilename, finalFilename);
  if (err == -1) {
    perror("rename");
    printf("Error renaming zipfile to finalFilename");
  }

  return 0;
}

int unlistFile(char *originalFilename) {
  int err;
  char filename[strlen(originalFilename) + 9];
  strcpy(filename, originalFilename);
  strcat(filename, ".gz.cpt.f");
  if (access(filename, F_OK) != 0) {
    printf("File isn't indexed\n");
    return -1;
  }
  FILE *file = fopen(filename, "r");
  int size = getFileSize(file);
  int times = size / 41;
  unsigned char hashes[times][41];
  int hashIdx, i;

  hashesFromFile(file, hashes, &hashIdx);

  for (i = 0; i < times; i++) {
    int type = 3;
    int code = 0;
    char location[20];
    server_t server = {0};
    int result = searchFileLocation(hashes[i], location);
    if (result != 1) {
      printf("No file found\n");
      return -1;
    }

    err = (server.listen_fd = socket(AF_INET, SOCK_STREAM, 0));
    if (err == -1) {
      perror("socket");
      printf("client: Failed to create socket endpoint\n");
      return err;
    }

    err = server_connect(&server, location, PORT);
    if (err == -1) {
      perror("connect");
      printf("Error connecting\n");
    }

    err = write(server.listen_fd, &type, sizeof(int));
    if (err == -1) {
      perror("write");
      printf("Error writing type\n");
    }

    err = write(server.listen_fd, hashes[i], 41);
    if (err == -1) {
      perror("write");
      printf("Error writing hash\n");
    }

    err = read(server.listen_fd, &code, sizeof(int));
    if (err == -1) {
      perror("read");
      printf("Error reading code\n");
    }

    if (code == 0) {
      printf("Error removing remote file\n");
    }
    connection_close(server.listen_fd);
  }
  err = remove(filename);
  if (err == -1) {
    perror("remove");
    printf("Error removing file\n");
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
        sprintf(filename, "%s/%s", dirname, event->name);

        if (event->mask & IN_CREATE) {
          printf("The file %s was created.\n", event->name);
          if (event->name[strlen(event->name) - 1] != 'f' ||
              event->name[strlen(event->name) - 2] != '.')
            handleFile(filename);
        } else if (event->mask & IN_DELETE) {
          printf("The file %s was deleted.\n", event->name);
        } else if (event->mask & IN_MODIFY) {
          printf("The file %s was modified.\n", event->name);
          if (event->name[strlen(event->name) - 1] != 'f' ||
              event->name[strlen(event->name) - 2] != '.') {
            unlistFile(filename);
            handleFile(filename);
          }
        } else if (event->mask & IN_ACCESS || event->mask & IN_OPEN) {
          if (event->name[strlen(event->name) - 1] == 'f' &&
              event->name[strlen(event->name) - 2] == '.') {
            printf("The file %s was accessed.\n", event->name);
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
