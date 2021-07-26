#include<math.h>
#include<stdio.h>
#include<dirent.h>
#include<stdlib.h>

void read_folder_files();
int main() {
  printf("%s", "asdfasdf");
  read_folder_files(".");
  return 0;
}

void read_folder_files(char dirname[10]) {
  DIR *folder;
  int idx = 0;
  struct dirent *entry;
  char *files[10];
  
  folder = opendir(dirname);
  if (folder == NULL) {
    puts("No dir");
  } else {
    while ((entry=readdir(folder))) {
      files[idx] = entry->d_name;
      idx++;
    }
  }

  for (idx = 0; idx < 10; idx++) {
    printf("File %d: %s\n", idx, files[idx]);
  }
}


