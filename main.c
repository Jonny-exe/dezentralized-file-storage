#include <dirent.h>
#include <math.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_FILE_SIZE 4294967296 // 4 gb

// Running gdb: gcc -Wall main.c -lcrypto -lm -o main
// Using it:    gcc -Wall -g main.c -lcrypto -lm -o main
//

void readFolderFiles();
int compressFile();
int cryptFile();
void splitFile2Bytes();
int tests();
int getFileSize();
unsigned char *hashFile();

int main() {
  /*tests();*/
  int err = cryptFile("a random key", "./test/testfiles", "decrypt");
  cryptFile("a random key", "./test/testfiles", "decrypt");
  if (err == -1) {
    puts("Error");
  }

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

  int idx, c, max;
  int bytesIdx = 0;
  int pieces = 16; //TODO make this dynamic
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
      printf("%d, %d, %d\n",myBytes[bytesIdx][idx], idx, bytesIdx);
    }
  }
  for (int i = 0; i < bytesIdx + 1; i++)
    bytes[i] = myBytes[i];

  printf("Last byteIdx: %d\n", bytesIdx);
  /*puts(""); //TODO: Why the heck is this important. Code doesn't work without this*/
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

unsigned char *hashFile(int *bytes, int bytesSize) {
  // TODO
  int i;
  unsigned char hash[SHA_DIGEST_LENGTH];
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
  return hash;
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

int tests() {
  char filename[30] = "./test/testfiles";
  FILE *file = fopen(filename, "rb");
  int size = getFileSize(file);
  int *bytes[256 / sizeof(int)];
  printf("Size of bytes: %d\n", (int) sizeof(bytes));
  printf("Size of file: %d\n", size);
  splitFile2Bytes(file, size, bytes);
  printf("Size of bytes: %d\n", (int) sizeof(bytes));
  for (int i = 0; i < 5; i++) {
    hashFile(bytes[i], 64);
  }
  return 0;
}
