#include <dirent.h>
#include <math.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 4294967296 // 4 gb

// gcc -Wall main.c -lcrypto -lm -o main

void readFolderFiles();
int compressFile();
int encryptFile();
void splitFile2Bytes();
int sendFile();
int tests();
int getFileSize();
char *hashFile();

int main() {
  char filename[30] = "./test/testfiles";
  FILE *file = fopen(filename, "rb");
  int size = getFileSize(file);
  int bytes[256][(int)ceil((double)size / (double)256)];
  printf("Size of bytes: %d || %d\n", (int)sizeof(bytes),
         (int)ceil((double)size / (double)256));
  splitFile2Bytes(file, size, bytes);
  return 0;
}

void readFolderFiles(char *dirname, char **files) {
  const int MAX_FILENAME_LENGTH = 10;
  DIR *folder;
  int idx = 0;
  struct dirent *entry;

  folder = opendir(dirname);
  if (folder == NULL) {
    puts("No dir\n");
  } else {
    while ((entry = readdir(folder))) {
      char *name = entry->d_name;
      if (name[0] == '.') {
        continue;
      }
      char *fullName;
      fullName = malloc(strlen(dirname) + strlen(name) + sizeof(char));
      strcpy(fullName, dirname);
      strcat(fullName, "/");
      strcat(fullName, name);

      if (entry->d_type == 4) {
        char *filesInsideFolder[MAX_FILENAME_LENGTH * sizeof(char)];
        readFolderFiles(fullName, filesInsideFolder);
        for (int i = 0;
             i < sizeof(filesInsideFolder) / MAX_FILENAME_LENGTH / sizeof(char);
             i++) {
          files[idx] = filesInsideFolder[i];
          idx++;
        }
      } else {
        files[idx] = fullName;
        idx++;
      }
      free(fullName);
    }
  }
}

int compressFile(char *filename) {
  char command[30] = "gzip ";
  int result;
  strcat(command, filename);
  result = system(command);
  return result;
}

void splitFile2Bytes(FILE *file, int size, int **bytes) {
  // TODO: fix this with huge files:
  // https://stackoverflow.com/questions/41859547/how-to-read-a-large-file-with-function-read-in-c?noredirect=1&lq=1
  if (file == NULL) {
    printf("File does not exist.");
    exit(1);
  }

  int currentBytes[256];
  int idx, c, max;
  int bytesIdx = 0;

  for (idx = 0, max = 0; max < size; idx++, max++) {
    c = getc(file);
    currentBytes[idx] = c;
    if (idx % 255 == 0 && idx != 0) {
      idx = 0;
      bytes[bytesIdx] = currentBytes;
      bytesIdx++;
    }
  }

  if (idx != 0) {
    for (idx = idx; idx < 256; idx++)
      currentBytes[idx] = 0;
    bytes[bytesIdx] = currentBytes;
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

char *hashFile(char *filename) {
  // TODO
  char hash[30 * sizeof(char)];
  return "0";
}

int encryptFile() {
  // TODO
  return 0;
}

int sendFile(int *file) {
  // TODO
  return 0;
}

int tests() {
  // TODO
  return 0;
}
