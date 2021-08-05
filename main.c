#include "helpers.c"
#include <stdio.h> 
#include <string.h>
#include <sys/inotify.h>
#define MAX_FILENAME 80
#define TEMP_FILE "temp.tar"

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
  char command[100];
  sprintf(command, "mv '%s' temp",  filename);
  err = system(command);
  err = compressFile("temp");
  if (err == -1)
    printf("Error compressing file\n");
  strcat(filename, ".gz");
  sprintf(command, "mv temp.gz '%s'",  filename);
  err = system(command);

  FILE *file = fopen(filename, "rb");

  int size = getFileSize(file);
  int times = ceil((double)size / (double)64);
  int bytes[times][256 / sizeof(int)];
  splitFile2Bytes(file, size, bytes, times);

  int j;
  fclose(file);

  unsigned char hashes[times][80];
  for (i = 0; i < times; i++) {
    hashFile(bytes[i], 64, hashes[i]);
  }
  err = cryptFile("a random key", filename, "encrypt");
  if (err == -1)
    printf("Error encrypting file\n");
  strcat(filename, ".cpt");

  createFile(filename);
  strcat(filename, ".f");
  file = fopen(filename, "w");
  for (i = 0; i < times; i++) {
    fprintf(file, "%s\n", hashes[i]);
  }
  return 0;
}
