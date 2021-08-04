#include "helpers.c"
#include <stdio.h> 
#include <string.h>
#include <sys/inotify.h>
#define MAX_FILENAME 80

int createFakeTree(char files[MAX_FILENAME][100], int size);
int handleFile(char *filename);
int main() {
  /*char files[MAX_FILENAME][100];*/
  /*char dirname[MAX_FILENAME] = "./test";*/
  /*int index;*/

  /*readFolderFiles(dirname, files, &index); */
  /*createFakeTree(files, index);*/
  /*listenFolder(".");*/
  char filename[100] = "./test/testfiles";
  handleFile(filename);
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
  err = compressFile(filename);
  if (err == -1)
    printf("Error compressing file\n");

  strcat(filename, ".gz");
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");

  strcat(filename, ".cpt");
  FILE *file = fopen(filename, "rb");

  int size = getFileSize(file);
  int times = ceil((double)size / (double)64);
  //int bytes[256 / sizeof(int)][times];
  int bytes[times][256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes, times);

  int j;
  for (i = 0; i < times; i++) {
    for (j = 0; j < 64; j++)
      printf("Test: %d\n", bytes[i][j]);
  }
  fclose(file);

  unsigned char hashes[41][times];
  for (int i = 0; i < times; i++) {
    hashFile(bytes[i], 64, hashes[i]);
  }

  strcat(filename, ".f");
  createFile(filename);
  file = fopen(filename, "w");
  return 0;
}
