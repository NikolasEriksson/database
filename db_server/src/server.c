#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>

#include <signal.h>
// for request lib
#include "../include/request.h"
// for input/output (db)
#include "../include/io.h"
// for waitpid
#include <sys/wait.h>
#define DIE(str) perror(str);exit(-1);
#define BUFSIZE 255

// when child terminates the parent gets signal and call this function to terminate the process
void sighandler(){ 
	while(waitpid(-1, NULL, WNOHANG) > 0){ // -1 = wait for any child, WNOHANG = don't block if the state isn't changed (man waitpid)
		continue;
	}

	puts("Child process terminated");
}




int main(int argc, char* argv[]) {
	int portnumber = 1337; // default port
	struct sockaddr_in sin, pin;
	int serverSocket; 
	int clientSocket;
	int addrlen;
	char buf[BUFSIZE];
	pid_t pid;
	char message[255];
	int isQuit=0;
	int showTables=0;
	int showSchema=0;
	
	if(argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
		exit(-1);
	}
	
	int i;
	for(i=0; i < argc; i++){
		if(strcmp(argv[i], "-h") == 0){
			printf("Usage: %s -p <port>\n", argv[0]);
			puts("-h print help text.");
			puts("-p listen to port number <port>.");
			exit(0);
			//puts("-d run as a daemon.\n");
			//puts("-l logfile, log to logfile. If unspecified, logging will be output to syslog.\n");
		}else if(strcmp(argv[i], "-p") == 0){
			if(argv[i+1] != NULL) portnumber = atoi(argv[i+1]);
			else{
				printf("Usage: %s -p <port>\n", argv[0]);
				exit(0);
			}
		}
	}

	printf("Server started\n------\n");
	printf("Port: %i\n", portnumber);
	puts("Request handeling method: Fork");
	puts("------");

        /* get a file descriptor for an IPv4 socket using TCP */
	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		DIE("socket");
	}

	int option = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        /* zero out the sockaddr_in struct */
	memset(&sin, 0, sizeof(sin));
        /* setup the struct to inform the operating system that we would like
         * to bind the socket the the given port number on any network
         * interface using IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(portnumber);

        /* perform bind call */
	if(bind(serverSocket, (struct sockaddr*) &sin, sizeof(sin)) == -1){
		DIE("bind");
	}
	
	listen(serverSocket, 10); // there wont be any connections waiting since we fork each new client. keep at 10.
	//shutdown(sd, SHUT_RDWR);
	addrlen = sizeof(pin);

	signal(SIGCHLD, sighandler);

	while(1){
		// accept connection
		clientSocket = accept(serverSocket, (struct sockaddr*) &pin, (socklen_t*) &addrlen);

		// clean the buffers, strange chars will appear in the server console otherwise.
		memset(buf, 0, sizeof buf);
		memset(message, 0, sizeof message);

		if((pid = fork()) == -1){ // something went very wrong
			close(clientSocket);
			close(serverSocket);
			continue;
		// the parent process doesn't need to to anything after this, since we're waiting for the SIGCHILD signal
		}else if(pid == 0){ // in child process
			close(serverSocket);
			while(1){
				do {					        
					// receive at most sizeof(buf) many bytes and store them in the buffer */
					if(recv(clientSocket, buf, sizeof(buf), 0) == -1) { 
						DIE("recv");
					}

					strcat(message, buf);

					// check if the buffer contains the different commands that will have to be executed instantly ( no need for ; )
					isQuit = strstr(buf, ".quit")	    ? 1 : 0;
					showTables = strstr(buf, ".tables") ? 1 : 0;
					showSchema = strstr(buf, ".schema") ? 1 : 0;
					// there was another memset here previosly, doesn't seem to be needed anymore
				}while(!strstr(message, ";") && (!isQuit) && (!showTables) && (!showSchema));
				
				/* convert IP address of communication partner to string */
				char ipAddress[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));
				
				// print the message to server terminal
				printf("%s:%i - %s\n", ipAddress, ntohs(pin.sin_port), message);

				// error handeling var
				char* error;				
				
				// create the request and parse it
				request_t *request;
				request = parse_request(message, &error);
				if(request == NULL){ // if there was an error with the request, tell the client and free the error
					strcat(error, "\n");
					send(clientSocket, error, strlen(error) + 1, 0);
					free(error);
				}

				if(isQuit){ // if the client sent .quit, close the connection and exit the process (and send the SIGCHLD signal)
					printf("Closing connection with %s:%i by request from client.\n", ipAddress, ntohs(pin.sin_port));
					destroy_request(request);
					shutdown(clientSocket, SHUT_RDWR);
					close(clientSocket);
					exit(pid);
				}else if(showTables){ // if the client sent .tables, show all tables if they exist, else tell the client that no tables exists
					char* returnTable = allTables();
					if(strcmp(returnTable, "empty") != 0) {
						send(clientSocket, "Showing all tables\n", strlen("Showing all tables\n") + 1, 0);
						send(clientSocket, returnTable, strlen(returnTable) + 1, 0);
					} else {
						send(clientSocket, "No tables exists\n", strlen("No tables exists\n") + 1, 0);
					}
					free(returnTable);
				}else if(showSchema){ // if the client sent .schema x, x=table, send the schema back or tell the client that the table doesn't exist
					char* returnSchema = tableSchema(request);
					if(strcmp(returnSchema, "Table does not exist") != 0) {					
						send(clientSocket, "Showing schema for table\n", strlen("Showing schema for table\n") + 1, 0);
						send(clientSocket, returnSchema, strlen(returnSchema) + 1, 0);
					} else {
						send(clientSocket, "No such table exists\n", strlen("No such table exists\n") +1, 0);
					}
					free(returnSchema);
				}else{ // if the request wasn't one of the .requests, check the request type 
					char* returnVal;
					switch(request->request_type){
						case RT_CREATE:
							returnVal = createTable(request);
							send(clientSocket, returnVal, strlen(returnVal)+1, 0);
							free(returnVal);
							break;
						case RT_DROP:
							returnVal = dropTable(request);
							send(clientSocket, returnVal, strlen(returnVal)+1, 0);
							free(returnVal);
							break;
						case RT_INSERT:
							returnVal = insert(request);
							send(clientSocket, returnVal, strlen(returnVal)+1, 0);
							free(returnVal);
							break;
						case RT_SELECT:
							returnVal = selectValues(request);
							send(clientSocket, returnVal, strlen(returnVal)+1, 0);
							free(returnVal);
							break;
						default:
							break;
					}
				}
				destroy_request(request); // DESTROY the request
	
				memset(buf, 0, sizeof buf);
				memset(message, 0, sizeof message);
			}
		} 
	}

	shutdown(serverSocket, SHUT_RDWR);
	close(serverSocket);
	exit(0);
}

