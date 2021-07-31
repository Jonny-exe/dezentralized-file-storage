#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#define SEPARATOR "||"
#define FILENAME "local_hash_table"

typedef struct table {
  char hash[80];
  char ip[80];
} table_t;

int update_table(FILE *file, char *new_key, char *new_value);
int insert_table(char *new_key, char *new_value);
int remove_table(char *key);
int init_table();
int read_table(table_t *table, int *lines);
int write_table(table_t *table, int lines);
void format_time(char *output);

int main() {
  printf("Main init\n");
  int err;
  err = init_table();
  if (err == -1) {
    printf("Error initializing file\n");
  }

  /*err = insert_table("key", "value");*/
  /*if (err == -1) {*/
    /*printf("Error inserting key\n");*/
  /*}*/

  table_t tables[80];
  int lines = 0;
  read_table(tables, &lines);
  /*for (int i = 0; i < lines; i++) {*/
    /*printf("%s, %s\n", tables[i].ip, tables[i].hash);*/
  /*}*/
  write_table(tables, lines);
  return 0;
}

int remove_table(char *key) {
  char ch;
  table_t lines[10];
  int idx = 0;
  FILE *file = fopen(FILENAME, "r");
  while((ch = fgetc(file)) != EOF) {
    if (ch == '\n') {
      
    }
  }

  return 0;
}

int init_table() {
  if (access(FILENAME, F_OK) != 0) {
    FILE *file = NULL;
    file = fopen(FILENAME, "w+");
    if (file == NULL) {
      return -1;
    }
    char date[20];
    format_time(date);
    fprintf(file, "%s", date);
    fclose(file);
  }
  return 0;
}

int read_table(table_t *tables, int *lines ) {
  char ch;
  int table_idx = -1;
  int separator_idx = 0;
  int idx = 0;
  int type = 0; // 0 == value, 1 == key
  char value[80];
  char key[80];
  FILE *file = fopen(FILENAME, "r");

  while((ch = fgetc(file)) != EOF) {
    if (ch == '\n') {
      if (table_idx > -1) {
        strcpy(tables[table_idx].hash, key);
        strcpy(tables[table_idx].ip, value);
        type = !type;
        idx = 0;
      }
      memset(value, 0, sizeof(value));
      memset(key, 0, sizeof(key));
      table_idx++;
      continue;
    } else if (table_idx > -1) {
      if (ch == SEPARATOR[separator_idx]) {
        separator_idx++;
        if (separator_idx == 2) {
          type = !type;
          idx = 0;
        }
      } else {
        separator_idx = 0;
        if (type) {
          key[idx] = ch;
        } else {
          value[idx] = ch;
        }
        idx++;
      }
    }
  }
  *lines = table_idx;
  return 0;
}

int write_table(table_t *tables, int lines) {
  if (lines == 0) {
    return -1;
  }

  int err;
  FILE *file = fopen(FILENAME, "w+");
  char date[20];
  format_time(date);

  fprintf(file, "%s\n", date);
  for (int i = 0; i < lines; i++) {
    fprintf(file, "%s||%s\n", tables[i].hash, tables[i].ip);
  }

  err = fclose(file);
  return err;
}

int insert_table(char *new_key, char *new_value) {
  int err;
  FILE *file = NULL;
  file = fopen(FILENAME, "a");
  fprintf(file, "%s%s%s\n", new_key, SEPARATOR, new_value);
  err = fclose(file);
  return err;
}

void format_time(char *output) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  sprintf(output, "[%d-%d-%d/%d]", timeinfo->tm_mday, timeinfo->tm_mon + 1,
          timeinfo->tm_year + 1900, timeinfo->tm_hour);
}


