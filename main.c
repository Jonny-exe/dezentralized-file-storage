#include <string.h>
#include <stdio.h>
#include <sys/inotify.h>
#include "helpers.c"
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
    (void) createFile(files[i]);
  }
  return 0;
}

int handleFile(char *filename) {
  int err;
  err = compressFile(filename);
  if (err == -1)
    printf("Error compressing file\n");;

  strcat(filename, ".gz");
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");
  
  strcat(filename, ".cpt");
  FILE *file = fopen(filename, "rb");
  int size = getFileSize(file);
  int *bytes[256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes);

  for (int i = 0; i < 4; i++) {
    printf("%d\n", i);
    hashFile(bytes[i], 64);
  }
  return 0;
}
