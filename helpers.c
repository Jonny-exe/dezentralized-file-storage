#include <dirent.h>
#include <math.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>

#define MAX_FILENAME 80
#define MAX_FILE_SIZE 4294967296 // 4 gb
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

// Running gdb: gcc -Wall -g main.c -lcrypto -lm -o main
// Using it:    gcc -Wall main.c -lcrypto -lm -o main

void readFolderFiles(char *dirname, char files[MAX_FILENAME][100], int *index);
int compressFile(char *filename);
void splitFile2Bytes(FILE *file, int size, int *bytes[256 / sizeof(int)]);
int cryptFile(char *key, char *filename, char *type);
int getFileSize(FILE *file);
int listenFolder(char *dirname);
void hashFile(int *bytes, int bytesSize, unsigned char *hash);
int tests();

void readFolderFiles(char *dirname, char files[MAX_FILENAME][100], int *index) {
  DIR *folder;
  int idx = *index;
  struct dirent *entry;

  folder = opendir(dirname);
  if (folder == NULL) {
    puts("No dir\n");
  } else {
    while ((entry = readdir(folder))) {
      char *name = entry->d_name;
      printf("Size: %d %s\n", (int)strlen(name), name);
      if (name[0] == '.' ||
          (name[strlen(name) - 1] == 'f' && name[strlen(name) - 2] == '.')) {
        continue;
      }
      char fullName[strlen(dirname) + strlen(name) + sizeof(char)];
      strcpy(fullName, dirname);
      strcat(fullName, "/");
      strcat(fullName, name);

      if (entry->d_type == 4) {
        readFolderFiles(fullName, files, &idx);
      } else {
        strcpy(files[idx], fullName);
        idx++;
      }
    }
  }
  *index = idx;
}

void splitFile2Bytes(FILE *file, int size, int *bytes[256 / sizeof(int)]) {
  // TODO: fix this with huge files:
  // https://stackoverflow.com/questions/41859547/how-to-read-a-large-file-with-function-read-in-c?noredirect=1&lq=1
  if (file == NULL) {
    printf("File does not exist.");
    exit(1);
  }

  int idx, c, max;
  int bytesIdx = 0;
  int pieces = ceil((double)size / (double)64); // TODO make this dynamic
  //int pieces = 16;
  int myBytes[256 / sizeof(int)][pieces];

  for (idx = 0, max = 0; max < size; idx++, max++) {
    c = getc(file);
    myBytes[bytesIdx][idx] = c;
    if (idx % 63 == 0 && idx != 0) {
      idx = -1;
      bytesIdx++;
    }
  }

  if (idx != 0) {
    for (idx = idx; idx < 256 / sizeof(int); idx++) {
      myBytes[bytesIdx][idx] = 0;
      printf("%d, %d, %d\n", myBytes[bytesIdx][idx], idx, bytesIdx);
    }
  }
  int i;
  for (i = 0; i < (bytesIdx + 1); i++) {
    bytes[i] = myBytes[i];
  }

  printf("Last byteIdx: %d\n", bytesIdx);
}

int getFileSize(FILE *file) {
  if (file == NULL) {
    puts("File was not found");
    exit(1);
  }

  fseek(file, 0L, SEEK_END);
  int size = ftell(file);

  if (size >= MAX_FILE_SIZE) {
    printf("File is too big.");
    exit(1);
  }
  fseek(file, 0L, SEEK_SET);
  return size;
}

void hashFile(int *bytes, int bytesSize, unsigned char *finalhash) {
  // TODO: maybe use something better
  int i;
  unsigned char *hash = malloc(SHA_DIGEST_LENGTH);;
  SHA_CTX mdContent;
  SHA1_Init(&mdContent);

  printf("Size of a: %d %d\n", (int)sizeof(&bytes), (int)sizeof(int));

  for (i = 0; i < bytesSize; i++) {
    char intString[5];
    sprintf(intString, "%d", bytes[i]);
    SHA1_Update(&mdContent, intString, sizeof(intString));
  }

  SHA1_Final(hash, &mdContent);
  printf("SHA LENGTH: %d\n", SHA_DIGEST_LENGTH * 2 + 1);
  int idx = 0;
  for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
    char piece[3];
    sprintf(finalhash + idx, "%02x", hash[i]);
    idx += 2;
  }
  finalhash[SHA_DIGEST_LENGTH * 2 + 1] = '\0';
}

int cryptFile(char *key, char *filename, char *type) {
  if (strcmp(type, "encrypt") != 0 && strcmp(type, "decrypt") != 0)
    return -1;

  char command[100];
  int result;
  sprintf(command, "ccrypt --%s --key '%s' '%s'", type, key, filename);
  result = system(command);
  return result;
}

int compressFile(char *filename) {
  char command[MAX_FILENAME] = "gzip ";
  int result;
  strcat(command, filename);
  result = system(command);
  return result;
}

int createFile(char *filename) {
  int err;
  char command[MAX_FILENAME * 3];
  sprintf(command,
          "file=\"%s\" && mkdir -p \"${file%%/*}\" && touch \"$file.f\"",
          filename);
  err = system(command);
  return err;
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
    wd = inotify_add_watch(fd, dirname, IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0) {
      perror("read");
    }

    while (i < length) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];
      if (event->len) {
        if (event->mask & IN_CREATE) {
          printf("The file %s was created.\n", event->name);
        } else if (event->mask & IN_DELETE) {
          printf("The file %s was deleted.\n", event->name);
        } else if (event->mask & IN_MODIFY) {
          printf("The file %s was modified.\n", event->name);
        }
      }
      i += EVENT_SIZE + event->len;
    }

    (void)inotify_rm_watch(fd, wd);
    (void)close(fd);
  }

  return 0;
}

int tests() {
  char filename[MAX_FILENAME] = "./test/testfiles";
  FILE *file = fopen(filename, "rb");
  int size = getFileSize(file);
  int bytes[256 / sizeof(int)][16];
  printf("Size of bytes: %d\n", (int)sizeof(bytes));
  printf("Size of file: %d\n", size);
  splitFile2Bytes(file, size, bytes);
  printf("Size of bytes: %d\n", (int)sizeof(bytes));
  unsigned char *hashes[SHA_DIGEST_LENGTH];
  for (int i = 0; i < 5; i++) {
    hashFile(bytes[i], 64, hashes[i]);
  }
  return 0;
}
