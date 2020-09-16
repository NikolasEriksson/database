#ifndef IO_H
#define IO_H
#include <string.h>
#include <stdio.h>
#include "request.h"

int fileexists(const char* filename) {
FILE *file;
	if((file = fopen(filename, "r"))) {
		fclose(file);
		return 1;
	}
	return 0;
}

char* create_table(request_t *request) {
// File pointers to the file to be created
FILE* all_tables;
FILE* table_schema;
FILE* table_content;

// Filename taken from the table name
char* filename = request->table_name;

// Path to the folder
char* path = "database//";
char* path2 = "database//Table_schema//";
char* path3 = "database//Table_contents//";

// Extension to make them with correct name
char* extension = "all_tables.txt";
char* extension2 = "_table_schema.txt";
char* extension3 = "_table_contents.txt";

// Get correct size of full path filename
char fullfile[strlen(path)+strlen(extension)+1];
char fullfile2[strlen(path2)+strlen(filename)+strlen(extension2)+1];
char fullfile3[strlen(path3)+strlen(filename)+strlen(extension3)+1];

//Concat filenames
snprintf( fullfile, sizeof( fullfile ), "%s%s", path, extension); // File for all tables
snprintf( fullfile2, sizeof( fullfile2 ), "%s%s%s", path2, filename, extension2); // File for the table schema
snprintf( fullfile3, sizeof( fullfile3 ), "%s%s%s", path3, filename, extension3); // File for the contents of the table

//Check if the file exists

if(fileexists(fullfile2) == 0) {
	//Open the files with correct filename
	all_tables = fopen(fullfile, "a");
	table_schema = fopen(fullfile2, "w");
	table_content = fopen(fullfile3, "w");

	//Print table name to to file
	fprintf(all_tables, "%s\n", request->table_name);
	//Set the first request column to be able to loop through 
	column_t *current = request->columns; 
			//Loop through all columns and add column name and type + char_size	
			while(current != NULL) {
				fprintf(table_schema, "%s\t", current->name);
				if (current->data_type == 0) {
					fprintf(table_schema, "INT");
				} else {
					fprintf(table_schema, "VARCHAR(%i)\n", current->char_size);
				}
					fprintf(table_schema, "\n");
					current = current->next;
				}
			
	fclose(table_schema);
	fclose(table_content);
	fclose(all_tables);
	return "Succesfully created\n";
	} 

	return "Table already exists\n";
}


void insert(request_t *request) {


}
/*
void select(request_t *request) {

}

void drop_table(request_t *request) {

}

void all_tables() {

}

void table_schema(request_t *request) {

}*/

// create table, insert into table, select from table, drop table

#endif
