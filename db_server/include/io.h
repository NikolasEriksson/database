#include <string.h>
#include <stdio.h>
#include "request.h"

void create_table(request_t *request) {
FILE* all_tables;
FILE* table_schema;
FILE* table_content;
char* filename = request->table_name;
char* extension = "all_tables.txt";
char* extension2 = "_table_schema.txt";
char* extension3 = "_table_contents.txt";
char fullfile[strlen(extension)+1];
char fullfile2[strlen(filename)+strlen(extension2)+1];
char fullfile3[strlen(filename)+strlen(extension3)+1];
snprintf( fullfile, sizeof( fullfile ), "%s" , extension); // File for all tables
snprintf( fullfile2, sizeof( fullfile2 ), "%s%s", filename, extension2); // File for the table schema
snprintf( fullfile3, sizeof( fullfile3 ), "%s%s", filename, extension3); // File for the contents of the table

all_tables = fopen(fullfile, "a");
table_schema = fopen(fullfile2, "w");
table_content = fopen(fullfile3, "w");

fputs(request->table_name, all_tables);
fputs("\n", all_tables);
fputs(request->columns->name, table_schema);

fclose(all_tables);
fclose(table_schema);
fclose(table_content);
}

/*void insert(request_t *request) {

}

void select(request_t *request) {

}

void drop_table(request_t *request) {

}

void all_tables() {

}

void table_schema(request_t *request) {

}*/

// create table, insert into table, select from table, drop table


