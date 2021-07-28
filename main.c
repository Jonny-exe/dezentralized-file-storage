#include <dirent.h>
#include <math.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

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
  tests();
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
        readFolderFiles(fullName, filesInsideFolder); for (int i = 0;
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

void splitFile2Bytes(FILE *file, int size, int *bytes[256 / sizeof(int)]) {
/*int **splitFile2Bytes(FILE *file, int size, int *bytes[256]) {*/
  // TODO: fix this with huge files:
  // https://stackoverflow.com/questions/41859547/how-to-read-a-large-file-with-function-read-in-c?noredirect=1&lq=1
  if (file == NULL) {
    printf("File does not exist.");
    exit(1);
  }

  /*int currentBytes[256 / (int)sizeof(int)];*/
  int idx, c, max;
  int bytesIdx = 0;
  /*int pieces = 8;*/
  int pieces = 8; //TODO make this dynamic
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
    for (idx = idx; idx < 256 / sizeof(int); idx++)
      myBytes[bytesIdx][idx] = c;
  }
  for (int i = 0; i < pieces; i++) 
    bytes[i] = myBytes[i];

  puts(""); //TODO: Why the heck is this important. Code doesn't work without this
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

char *hashFile(int *bytes, FILE *file, int bytesSize) {
  // TODO
  int i;
  unsigned char hash[SHA_DIGEST_LENGTH];
  unsigned char data[1024];
  SHA_CTX mdContent;
  SHA1_Init(&mdContent);

  printf("Size of a: %d %d\n", (int)sizeof(&bytes), (int)sizeof(int));

  // while((bytes = fread(data, 1, 1024, file)))
  for (i = 0; i < bytesSize; i++) {
    printf("Test: %d, Idx: %d\n", bytes[i], i);
    SHA1_Update(&mdContent, &bytes[i], sizeof(&bytes[i]));
  }

  SHA1_Final(hash, &mdContent);
  for (i = 0; i < SHA_DIGEST_LENGTH; i++) printf("%02x", hash[i]);
  puts("\n");
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
  char filename[30] = "./test/testfiles";
  FILE *file = fopen(filename, "rb");
  int size = getFileSize(file);
  int *bytes[256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes);
  printf("Byte: %d\n", bytes[0][0]);
  for (int i = 0; i < sizeof(bytes) / sizeof(int) / (256 / sizeof(int)); i++) {
    hashFile(bytes[i], file, 64);
  }
  puts("HELLO");
  return 0;
}
