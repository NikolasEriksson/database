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

//int counter = 0;
// when child terminates the parent gets signal and call this function to terminate the process
void sighandler(){ 
	while(waitpid(-1, NULL, WNOHANG) > 0){ // -1 = wait for any child, WNOHANG = don't block if the state isn't changed (man waitpid)
		continue;
	}
	//counter--;
	//printf("counter: %i\n", counter);
	/*sleep(2);
	puts("Child process terminated");
	if(counter == 0) {
		puts("terminate parent");
		exit(0);	
	}*/
}




int main(int argc, char* argv[]) {
	int portnumber = 1337; // default port
	struct sockaddr_in sin, pin;
	int serverSocket; 
	int clientSocket;
	int addrlen;
	char buf[BUFSIZE];
	pid_t pid;

	if(argc < 1 || argc > 3) {
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

	addrlen = sizeof(pin);

	signal(SIGCHLD, sighandler);

	while(1){
		// accept connection
		clientSocket = accept(serverSocket, (struct sockaddr*) &pin, (socklen_t*) &addrlen);
		//counter++;
		// clean the buffer, strange chars will appear in the server console otherwise.
		memset(buf, 0, sizeof buf);

		if((pid = fork()) == -1){ // something went wrong
			close(clientSocket);
			close(serverSocket);
			continue;
		}else if(pid == 0){ // in child process // the parent process doesn't need to to anything after this, since we're waiting for the SIGCHILD signal
			close(serverSocket);
			while(1){
	
				// receive at most sizeof(buf) many bytes and store them in the buffer 
				if(recv(clientSocket, buf, sizeof(buf), 0) == -1) { 
						DIE("recv");
				}
				
				/* convert IP address of communication partner to string */
				char ipAddress[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));
				
				// print the buf to server terminal
				printf("%s:%i - %s\n", ipAddress, ntohs(pin.sin_port), buf);

				// error handeling var
				char* error;				
				
				// create the request and parse it
				request_t *request;
				request = parse_request(buf, &error);
				if(request == NULL){ // if there was an error with the request, tell the client and free the error
					strcat(error, "\n");
					send(clientSocket, error, strlen(error) + 1, 0);
					free(error);
					shutdown(clientSocket, SHUT_RDWR);
					close(clientSocket);
					exit(0);					
				}

				char* returnVal;
				switch(request->request_type){
					case RT_TABLES:
						returnVal = allTables();
						send(clientSocket, returnVal, strlen(returnVal)+1, 0);
						free(returnVal);
						break;
					case RT_SCHEMA:
						returnVal = tableSchema(request);
						send(clientSocket, returnVal, strlen(returnVal)+1, 0);
						free(returnVal);
						break;
					case RT_QUIT:
						printf("Closing connection with %s:%i by request from client.\n", ipAddress, ntohs(pin.sin_port));
						shutdown(clientSocket, SHUT_RDWR);
						close(clientSocket);
						exit(0);
						break;
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
				
				destroy_request(request); // DESTROY the request
	
				memset(buf, 0, sizeof buf);
			}
		}
	}

	shutdown(serverSocket, SHUT_RDWR);
	close(serverSocket);
	exit(0);
}

