#ifndef IO_H
#define IO_H
#include <string.h>
#include <stdio.h>
#include "request.h"
#include <dirent.h> // list all files in a dir
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>

int fileExists(const char* fileName) {
	FILE *file;
	if((file = fopen(fileName, "r"))) {
		fclose(file);
		return 1; // The file exists
	}
	return 0; // The file does not exist
}

char* createTable(request_t *request) {
	// File pointers to the file to be created
	FILE* allTables;
	FILE* tableSchema;
	FILE* tableContent;

	// Return value
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);
	
	// Variables to loop through files to check if there is an empty file
	char line[255];
	char* checkEmpty = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);
	
	char* fileName = "database/all_tables.txt";

	char fileName_2[255] = "database/Table_schema/";
	strncat(fileName_2, request->table_name, strlen(request->table_name));
	strncat(fileName_2, "_table_schema.txt", strlen("_table_schema.txt"));

	char fileName_3[255] = "database/Table_contents/";
	strncat(fileName_3, request->table_name, strlen(request->table_name));
	strncat(fileName_3, "_table_contents.txt", strlen("_table_contents.txt"));
	
	if(fileExists(fileName_2) == 0) { //Check if the file exists

	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	int lock1 = -1;

		while(lock1 != 0){
			allTables = fopen(fileName, "a");
			lock1 = fcntl(fileno(allTables), F_SETLK, &lock);
		}		
			tableSchema = fopen(fileName_2, "a");
			tableContent = fopen(fileName_3, "a");

		while(fgets(line, sizeof(line), tableSchema) != NULL){ // read each line of the provided file in the file variable
				strcat(checkEmpty, line);
			}
		free(checkEmpty);	
		fseek(tableSchema, 0, SEEK_END);
		if (ftell(tableSchema) == 0) {
			fprintf(allTables, "%s\n", request->table_name); //Print table name to to file 
			column_t *current = request->columns; //Set the first request column to be able to loop through 
					
			while(current != NULL) { //Loop through all columns and add column name and type + char_size
				fprintf(tableSchema, "%s\t", current->name);

				if (current->data_type == 0) {
					fprintf(tableSchema, "INT");
					} else {
						fprintf(tableSchema, "VARCHAR(%i)", current->char_size);
					}

				fprintf(tableSchema, "\n");
				current = current->next;
			}
			lock.l_type = F_UNLCK;
			fcntl(fileno(allTables), F_SETLK, &lock);
			strcat(ret, "Succesfully created\n");
		} else { 
			strcat(ret, "Table already exists\n");
		}	
			fclose(tableSchema);
			fclose(tableContent);
			fclose(allTables);
				 
	} else {
		strcat(ret, "Table already exists\n");
	}
	return ret;
}

char* insert(request_t *request) {
	FILE* tableContent;
	char fileName[255] = "database/Table_contents/";
	strncat(fileName, request->table_name, strlen(request->table_name));
	strncat(fileName, "_table_contents.txt", strlen("_table_contents.txt"));

	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);
	
	if(fileExists(fileName) == 1) {
		tableContent = fopen(fileName, "a");
		struct flock lock;
		memset(&lock, 0, sizeof(lock));
		lock.l_type = F_WRLCK;
		int lock1 = fcntl(fileno(tableContent), F_SETLK, &lock);
			if(lock1 != -1) {
				column_t *current = request->columns; 
				//Loop through all columns and add column name and type + char_size	
				while(current != NULL) {
					if (current->data_type == 0) {
						fprintf(tableContent, "%i\t", current->int_val);
					} else {
						fprintf(tableContent, "%s\t", current->char_val);
					}
						current = current->next;
				}  
			} else {
				insert(request);
			}
			fprintf(tableContent, "\n");
			lock.l_type = F_UNLCK;
			fcntl(fileno(tableContent), F_SETLK, &lock);
			fclose(tableContent);
			strcat(ret, "Successfully updated\n");
		} else {
			strcat(ret, "Table does not exist\n");
		}
	return ret;
}

char* selectValues(request_t *request) {

	FILE* tableContent;
	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret); // memset the ret string, it will contain weird chars in ret[0] otherwise

	char fileName[255] = "database/Table_contents/";
	strncat(fileName, request->table_name, strlen(request->table_name));
	strncat(fileName, "_table_contents.txt", strlen("_table_contents.txt"));

	if(fileExists(fileName) == 1) {
		tableContent = fopen(fileName, "r");
		while(fgets(line, sizeof(line), tableContent) != NULL){ // read each line of the provided file in the file variable
			strcat(ret, line);
		}
		fseek(tableContent, 0, SEEK_END);
		if (ftell(tableContent) == 0) {
			strcat(ret, "No entries in table\n");
		}
		fclose(tableContent);		
	} else {
		strcat(ret, "Table does not exist\n");
	}
	return ret;
}


char* dropTable(request_t *request) {
	char* fileName = "database/all_tables.txt";
	char* tempFileName = "database/all_tables_temp.txt";

	FILE* allTables;
	FILE* tempFile;

	int found = 0;
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if(fileExists(fileName) == 1){

		struct flock lock;
		memset(&lock, 0, sizeof(lock));
		lock.l_type = F_WRLCK; 
		int lock1 = -1;
		
		while(lock1 != 0){
			allTables = fopen(fileName, "r");
			tempFile = fopen(tempFileName, "w");
			lock1 = fcntl(fileno(tempFile), F_SETLK, &lock);
		}	
			char line[255];
			char* pos;
			while(fgets(line, sizeof(line), allTables) != NULL){ // read each line of the provided file in the file variable
				if((pos=strchr(line, '\n')) != NULL) *pos = '\0';
				if(strcmp(line, request->table_name) != 0){ // if the current line is NOT the table to be deleted		
					fprintf(tempFile, "%s\n", line);
					fflush(tempFile);
				}else{
					found = 1;
				}
			}
	
			if(found == 1){
				char first[255] = "database/Table_contents/";
				strncat(first, request->table_name, strlen(request->table_name));
				strncat(first, "_table_contents.txt", strlen("_table_contents.txt"));

				char second[255] = "database/Table_schema/";
				strncat(second, request->table_name, strlen(request->table_name));
				strncat(second, "_table_schema.txt", strlen("_table_schema.txt"));

				remove(first);
				remove(second);
				remove(fileName);
				rename(tempFileName, fileName);
				strcat(ret, "Table dropped\n");	
			}else{
				strcat(ret, "No such table\n");
			}

			lock.l_type = F_UNLCK;
			fcntl(fileno(tempFile), F_SETLK, &lock);

			fclose(allTables);
			fclose(tempFile);
				
	}else{
		strcat(ret, "No tables in the database\n");
	}
	return ret;
}

char* allTables() {
	char* fileName = "database/all_tables.txt";
	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if (fileExists(fileName) == 1) {
		FILE* allTables = fopen(fileName, "r");
		while(fgets(line, sizeof(line), allTables) != NULL){ // read each line of the provided file in the file variable
				strcat(ret, line);
			}	
		fseek(allTables, 0, SEEK_END);
		if (ftell(allTables) == 0) {
			strcat(ret, "empty");
		}				
		fclose(allTables);
	} else {
		strcat(ret, "empty");
	}

	return ret;
}


char* tableSchema(request_t *request) {
	char fileName[255] = "database/Table_schema/";
	strcat(fileName, request->table_name);
	strcat(fileName, "_table_schema.txt");

	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if (fileExists(fileName) == 1) {
		FILE* tableSchema = fopen(fileName, "r");
		while(fgets(line, sizeof(line), tableSchema) != NULL){ // read each line of the provided file in the file variable
			strcat(ret, line);
		}
		fclose(tableSchema);
	} else {
		strcat(ret, "Table does not exist\n");
	}

	return ret;
}

#endif
