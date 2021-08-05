#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#define SEPARATOR "||"
#define MAX_SIZE 200

typedef struct row {
  char hash[80];
  char ip[80];
} row_t;

int init_table(char *filename);
int update_row(row_t *table, char *key, char *new_value, int lines);
int insert_row(row_t *table, char *new_key, char *new_value, int *lines);
int remove_row(row_t *table, char *key, int lines);
int read_table(char *filename, row_t *table, int *lines);
int write_table(char *filename, row_t *table, int lines);
void format_date(char *output);
int hash_exists(row_t *table, char *hash, int lines);

int test() {
  int err;
  char filename[100] = "local_hash_table";
  err = init_table(filename);
  if (err == -1) {
    printf("Error initializing file\n");
  }

  row_t table[MAX_SIZE];
  int lines;
  err = read_table(filename, table, &lines);
  if (err == -1)
    printf("Error reading table\n");

  err = insert_row(table, "test1", "test1", &lines);
  if (err == -1)
    printf("Error inserting key\n");

  err = remove_row(table, "key", lines);
  if (err == -1)
    printf("Error removing key\n");

  err = update_row(table, "test1", "test2", lines);
  if (err == -1)
    printf("Error updating key\n");

  err = write_table(filename, table, lines);
  if (err == -1) 
    printf("Error writing table\n");
  return 0;
}

int init_table(char *filename) {
  if (access(filename, F_OK) != 0) {
    FILE *file = NULL;
    file = fopen(filename, "w+");
    if (file == NULL) {
      return -1;
    }
    char date[20];
    format_date(date);
    fprintf(file, "%s", date);
    fclose(file);
  }
  return 0;
}

int read_table(char *filename, row_t *table, int *lines ) {
  char ch;
  int table_idx = -1;
  int separator_idx = 0;
  int idx = 0;
  int type = 1; // 0 == value, 1 == key
  char value[80];
  char key[80];
  FILE *file = fopen(filename, "r");

  while((ch = fgetc(file)) != EOF) {
    if (ch == '\n') {
      if (table_idx > -1) {
        strcpy(table[table_idx].hash, key);
        strcpy(table[table_idx].ip, value);
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
        if (type == 1) {
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

int write_table(char *filename, row_t *table, int lines) {
  if (lines == 0) {
    return -1;
  }

  int err;
  FILE *file = fopen(filename, "w+");
  char date[20];
  format_date(date);

  fprintf(file, "%s\n", date);
  for (int i = 0; i < lines; i++) {
    if (strcmp(table[i].hash, "") == 0 && strcmp(table[i].ip, "") == 0)
      continue;
    fprintf(file, "%s||%s\n", table[i].hash, table[i].ip);
  }

  err = fclose(file);
  return err;
}

int hash_exists(row_t *table, char *hash, int lines) {
  for (int i = 0; i < lines; i++) {
    if (strcmp(table[i].hash, hash) == 0) {
      strcpy(table[i].ip, hash);
      return i;
    }
  }
  return -1;
}

int insert_row(row_t *table, char *new_key, char *new_value, int *lines) {
  int err;
  int line;

  err = -1;
  if (hash_exists(table, new_key, *lines) == -1) {
    strcpy(table[*lines].hash, new_key);
    strcpy(table[*lines].ip, new_value);
    (*lines)++;
    err = 0;
  }
  return err;
}

int update_row(row_t *table, char *key, char *new_value, int lines) {
  int line;

  line = hash_exists(table, key, lines);
  if (line == -1) {
    return -1;
  }

  strcpy(table[line].ip, new_value);
  return 0;
}

int remove_row(row_t *table, char *key, int lines) {
  int err = -1;
  for (int i = 0; i < lines; i++) {
    if (strcmp(key, table[i].hash) == 0) {
      strcpy(table[i].ip, "");
      strcpy(table[i].hash, "");
      err = 0;
      break;
    }
  }
  return err;
}

void format_date(char *output) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  sprintf(output, "[%d-%d-%d/%d]", timeinfo->tm_mday, timeinfo->tm_mon + 1,
          timeinfo->tm_year + 1900, timeinfo->tm_hour);
}


