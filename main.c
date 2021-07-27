#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readFolderFiles();
int compressFile();

int main() {
  char files[10 * sizeof(char)][100];
  readFolderFiles(".", files);
  char filename[20] = "./test/testfiles";
  compressFile(filename);
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
