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
#define BLOCK_SIZE (256/sizeof(int))


// Running gdb: gcc -Wall -g main.c -lcrypto -lm -o main
// Using it:    gcc -Wall main.c -lcrypto -lm -o main

void readFolderFiles(char *dirname, char files[100][MAX_FILENAME], int *index);
int compressFile(char *filename);
void splitFile2Bytes(FILE *file, int size, int *bytes, int times);
int cryptFile(char *key, char *filename, char *type);
int getFileSize(FILE *file);
void hashFile(int *bytes, int bytesSize, unsigned char *hash);
int tests();
int createFile(char *filename);
int removeFile(char *filename);
void hashesFromFile(FILE *file, char *hashes, int *hashIdx);

void readFolderFiles(char *dirname, char files[100][MAX_FILENAME], int *index) {
  DIR *folder;
  int idx = *index;
  struct dirent *entry;

  folder = opendir(dirname);
  if (folder == NULL) {
    puts("No dir\n");
  } else {
    while ((entry = readdir(folder))) {
      char *name = entry->d_name;
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

//void splitFile2Bytes(FILE *file, int size, int **bytes, int times) {
void splitFile2Bytes(FILE *file, int size, int *bytes, int times) {
  // TODO: fix this with huge files:
  // https://stackoverflow.com/questions/41859547/how-to-read-a-large-file-with-function-read-in-c?noredirect=1&lq=1
  if (file == NULL) {
    printf("File does not exist.");
    exit(1);
  }

  int idx, c, max;
  int bytesIdx = 0;

  for (idx = 0, max = 0; max < size; idx++, max++) {
    c = getc(file);
    bytes[bytesIdx * BLOCK_SIZE + idx] = c;
    if (idx % 63 == 0 && idx != 0) {
      idx = -1;
      bytesIdx++;
    }
  }

  if (idx != 0) {
    for (idx = idx; idx < BLOCK_SIZE; idx++) {
      bytes[bytesIdx * BLOCK_SIZE + idx] = 0;
    }
  }
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
  int i, idx = 0;
  unsigned char hash[SHA_DIGEST_LENGTH];

  char byteArray[64];
  memcpy(byteArray, bytes, 64);
  SHA1(byteArray, 64, hash);

  for (i = 0; i < SHA_DIGEST_LENGTH; i++, idx += 2) {
    sprintf(finalhash + 2 * i, "%02x", hash[i]);
  }
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
  char command[200];
  int result;
  sprintf(command, "touch -t 10161000 '%s' && gzip '%s'", filename, filename);
  result = system(command);
  return result;
}

int uncompressFile(char *filename) {
  char command[100];
  int result;
  sprintf(command, "gunzip -f '%s'", filename);
  result = system(command);
  return result;
}

int createFile(char *filename) {
  int err;
  char command[MAX_FILENAME * 3];
  sprintf(command,
          "file=\"%s\" && mkdir -p \"${file%%/*}\" && touch \"$file\"",
          filename);
  err = system(command);
  return err;
}

int removeFile(char *filename) {
  int err;
  char command[MAX_FILENAME * 3];
  sprintf(command, "rm '%s'", filename);
  err = system(command);
  return err;
}


void hashesFromFile(FILE *file, char *hashes, int *hashIdx) {
  if (file == NULL) {
    printf("File does not exist\n");
    return;
  }
  int idx = 0, hsIdx = 0;

  int ch;
  while((ch = fgetc(file)) != EOF) {
    if (ch == '\n') {
      hashes[hsIdx * 41 + idx] = '\0';
      idx = 0;
      hsIdx++;
      continue;
    }
    hashes[hsIdx * 41 + idx] = ch;
    idx++;
  }
  
  *hashIdx = hsIdx;
}

int tests() {
  /*
  char filename[MAX_FILENAME] = "./test/testfiles";
  FILE *file = fopen(filename, "rb");

  int size = getFileSize(file);
  int times = ceil((double)size / (double)64);
  int bytes[BLOCK_SIZE][times];

  splitFile2Bytes(file, size, bytes, times);
  unsigned char *hashes[SHA_DIGEST_LENGTH];
  for (int i = 0; i < 5; i++) {
    hashFile(bytes[i], 64, hashes[i]);
  }
  */
  char hashes[2][50];
  int i;
  hashesFromFile("test/asdfasdf.gz.cpt.f", hashes, &i);
  printf("hashes: %s, %s\n", hashes[0], hashes[1]);
  return 0;
}
