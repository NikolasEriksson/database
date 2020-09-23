#ifndef IO_H
#define IO_H
#include <string.h>
#include <stdio.h>
#include "request.h"
#include <dirent.h> // list all files in a dir
#include <stdbool.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>

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

// Return value
char* ret = malloc(sizeof(char)*255);
memset(ret, 0, sizeof ret);

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
					fprintf(table_schema, "VARCHAR(%i)", current->char_size);
				}
					fprintf(table_schema, "\n");
					current = current->next;
				}
			
	fclose(table_schema);
	fclose(table_content);
	fclose(all_tables);
	ret = "Succesfully created\n";
	} else {
		ret = "Table already exists\n";
	}

	return ret;
}

char* insert(request_t *request) {

FILE* table_content;
char* filename = malloc(sizeof(char)*255);
memset(filename, 0, sizeof filename);
strcat(filename, "database/Table_contents/");
strcat(filename, request->table_name);
strcat(filename, "_table_contents.txt");
char* ret = malloc(sizeof(char)*255);
memset(ret, 0, sizeof ret);

if(fileexists(filename) == 1) {
	table_content = fopen(filename, "a");
	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	int test1 = fcntl(fileno(table_content), F_SETLK, &lock);
	
		if(test1 != -1) {
			column_t *current = request->columns; 
			//Loop through all columns and add column name and type + char_size	
			while(current != NULL) {
				if (current->data_type == 0) {
					fprintf(table_content, "%i\t", current->int_val);
				} else {
					fprintf(table_content, "%s\t", current->char_val);
				}
					current = current->next;
			}  
		} else {
			insert(request);
		}
		fprintf(table_content, "\n");
		lock.l_type = F_UNLCK;
		fcntl(fileno(table_content), F_SETLK, &lock);
		fclose(table_content);
		ret = "Successfully updated\n";
	} else {
		ret = "Table does not exist\n";
	}
	return ret;
}

char* select_values(request_t *request) {

FILE* table_content;
char line[255];
char* ret = malloc(sizeof(char)*255);
memset(ret, 0, sizeof ret); // memset the ret string, it will contain weird chars in ret[0] otherwise
char* filename = malloc(sizeof(char)*255);
memset(filename, 0, sizeof filename);
strcat(filename, "database/Table_contents/");
strcat(filename, request->table_name);
strcat(filename, "_table_contents.txt");

	if(fileexists(filename) == 1) {
		table_content = fopen(filename, "r");
		while(fgets(line, sizeof(line), table_content) != NULL){ // read each line of the provided file in the file variable
			strcat(ret, line);
		}
		fseek(table_content, 0, SEEK_END);
		if (ftell(table_content) == 0) {
			ret = "No entries in table\n";
		}
		fclose(table_content);		
	} else {
		ret = "Table does not exist\n";
	}
	return ret;
}


char* drop_table(request_t *request) {
	char* fileName = "database/all_tables.txt";
	char* tempFileName = "database/all_tables_temp.txt";
	FILE* all_tables = fopen(fileName, "r");
	FILE* tempFile = fopen(tempFileName, "w");
	bool found = false;
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);
	
	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	int lock1 = fcntl(fileno(tempFile), F_SETLK, &lock);

	struct flock lock_2;
	memset(&lock_2, 0, sizeof(lock_2));
	lock_2.l_type = F_RDLCK;
	int lock2 = fcntl(fileno(all_tables), F_SETLK, &lock_2);

		
	if(lock1 != -1 && lock2 != -1){
		char line[255];
		char* pos;
		while(fgets(line, sizeof(line), all_tables) != NULL){ // read each line of the provided file in the file variable
			if((pos=strchr(line, '\n')) != NULL) *pos = '\0';
			if(strcmp(line, request->table_name) != 0){ // if the current line is NOT the table to be deleted		
				fprintf(tempFile, "%s\n", line);
				fflush(tempFile);
			}else{
				found = true;
			}
		}
	
		if(found){
			char first[255] = "database/Table_contents/";
			strcat(first, request->table_name);
			strcat(first, "_table_contents.txt");

			char second[255] = "database/Table_schema/";
			strcat(second, request->table_name);
			strcat(second, "_table_schema.txt");

			remove(first);
			remove(second);
			remove(fileName);
			rename(tempFileName, fileName);
			
			lock.l_type = F_UNLCK;
			lock_2.l_type = F_UNLCK;

			fcntl(fileno(tempFile), F_SETLK, &lock);
			fcntl(fileno(all_tables), F_SETLK, &lock_2);
			fclose(all_tables);
			fclose(tempFile);
			strcat(ret, "Table dropped\n");	
		}else{
			strcat(ret, "No such table\n");
		}
	}else{
		drop_table(request);
	}

	return ret;
}


char* all_tables() {
	char* fileName = "database/all_tables.txt";
	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if (fileexists(fileName) == 1) {
		FILE* all_tables = fopen(fileName, "r");
		while(fgets(line, sizeof(line), all_tables) != NULL){ // read each line of the provided file in the file variable
				strcat(ret, line);
			}	
		fseek(all_tables, 0, SEEK_END);
		if (ftell(all_tables) == 0) {
			strcat(ret, "empty");
		}				
		fclose(all_tables);
	} else {
		strcat(ret, "empty");
	}

	return ret;
}


char* table_schema(request_t *request) {
	char fileName[255] = "database/Table_schema/";
	strcat(fileName, request->table_name);
	strcat(fileName, "_table_schema.txt");

	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if (fileexists(fileName) == 1) {
		FILE* table_schema = fopen(fileName, "r");
		while(fgets(line, sizeof(line), table_schema) != NULL){ // read each line of the provided file in the file variable
			strcat(ret, line);
		}
		fclose(table_schema);
	} else {
		strcat(ret, "Table does not exist\n");
	}

	return ret;
}

#endif
