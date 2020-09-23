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

	//char* filename = request->table_name; // Filename taken from the table name
	
	char filename[255] = "database/all_tables.txt";

	char filename_2[255] = "database/Table_schema/";
	strcat(filename_2, request->table_name);
	strcat(filename_2, "_table_schema.txt");

	char filename_3[255] = "database/Table_contents/";
	strcat(filename_3, request->table_name);
	strcat(filename_3, "_table_contents.txt");
	
	if(fileexists(filename_2) == 0) { //Check if the file exists

	all_tables = fopen(filename, "a");
	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	int lock1 = fcntl(fileno(all_tables), F_SETLK, &lock);

		if(lock1 != -1) {
			sleep(5);
			table_schema = fopen(filename_2, "w");
			table_content = fopen(filename_3, "w");
			fprintf(all_tables, "%s\n", request->table_name); //Print table name to to file 
			column_t *current = request->columns; //Set the first request column to be able to loop through 
						
			while(current != NULL) { //Loop through all columns and add column name and type + char_size
				fprintf(table_schema, "%s\t", current->name);

				if (current->data_type == 0) {
					fprintf(table_schema, "INT");
					} else {
						fprintf(table_schema, "VARCHAR(%i)", current->char_size);
					}

				fprintf(table_schema, "\n");
				current = current->next;
			}
			
			lock.l_type = F_UNLCK;
			fcntl(fileno(all_tables), F_SETLK, &lock);
			fclose(table_schema);
			fclose(table_content);
			fclose(all_tables);
			strcat(ret, "Succesfully created\n");	
		} else {
			sleep(1);
			puts("File locked");
			create_table(request);
		}
	} else {
		strcat(ret, "Table already exists\n");
	}
	printf("%s", ret);
	return ret;
}

char* insert(request_t *request) {
	FILE* table_content;
	char filename[255] = "database/Table_contents/";
	strcat(filename, request->table_name);
	strcat(filename, "_table_contents.txt");

	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);
	
	if(fileexists(filename) == 1) {
		table_content = fopen(filename, "a");
		struct flock lock;
		memset(&lock, 0, sizeof(lock));
		lock.l_type = F_WRLCK;
		int lock1 = fcntl(fileno(table_content), F_SETLK, &lock);
			if(lock1 != -1) {
				sleep(5);
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
			strcat(ret, "Successfully updated\n");
		} else {
			strcat(ret, "Table does not exist\n");
		}
	return ret;
}

char* select_values(request_t *request) {

	FILE* table_content;
	char line[255];
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret); // memset the ret string, it will contain weird chars in ret[0] otherwise

	char filename[255] = "database/Table_contents/";
	strcat(filename, request->table_name);
	strcat(filename, "_table_contents.txt");

	if(fileexists(filename) == 1) {
		table_content = fopen(filename, "r");
		while(fgets(line, sizeof(line), table_content) != NULL){ // read each line of the provided file in the file variable
			strcat(ret, line);
		}
		fseek(table_content, 0, SEEK_END);
		if (ftell(table_content) == 0) {
			//ret = "No entries in table\n";
			strcat(ret, "No entries in table\n");
		}
		fclose(table_content);		
	} else {
		//ret = "Table does not exist\n";
		strcat(ret, "Table does not exist\n");
	}
	return ret;
}


char* drop_table(request_t *request) {
	char* fileName = "database/all_tables.txt";
	char* tempFileName = "database/all_tables_temp.txt";

	FILE* all_tables;
	FILE* tempFile;

	bool found = false;
	char* ret = malloc(sizeof(char)*255);
	memset(ret, 0, sizeof ret);

	if(fileexists(fileName) == 1){
		struct flock lock_2;
		memset(&lock_2, 0, sizeof(lock_2));
		lock_2.l_type = F_WRLCK;
		int lock2 = -1;

		struct flock lock;
		memset(&lock, 0, sizeof(lock));
		lock.l_type = F_WRLCK; 
		int lock1 = -1;
		
		while(lock1 != 0 && lock2 != 0){
			sleep(2);
			all_tables = fopen(fileName, "r");
			tempFile = fopen(tempFileName, "w");
			printf("1: lock1: %i, lock2: %i\n", lock1, lock2);
			lock1 = fcntl(fileno(tempFile), F_SETLK, &lock);
			lock2 = fcntl(fileno(all_tables), F_SETLK, &lock_2);
			printf("2: lock1: %i, lock2: %i\n", lock1, lock2);
		}
		
			char line[255];
			char* pos;
			while(fgets(line, sizeof(line), all_tables) != NULL){ // read each line of the provided file in the file variable
				if((pos=strchr(line, '\n')) != NULL) *pos = '\0';
				if(strcmp(line, request->table_name) != 0){ // if the current line is NOT the table to be deleted		
					fprintf(tempFile, "%s\n", line);
					fflush(tempFile);
				}else{
					found = true;
					puts("table existed");
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
				strcat(ret, "Table dropped\n");	
			}else{
				strcat(ret, "No such table\n");
			}

			lock.l_type = F_UNLCK;
			lock_2.l_type = F_UNLCK;

			fcntl(fileno(tempFile), F_SETLK, &lock);
			fcntl(fileno(all_tables), F_SETLK, &lock_2);

			fclose(all_tables);
			fclose(tempFile);
				
	}else{
		strcat(ret, "No tables in the database\n");
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
