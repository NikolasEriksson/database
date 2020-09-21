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
#include <stdbool.h>

// for request lib
#include "../include/request.h"
// for input/output (db)
#include "../include/io.h"
// for waitpid
#include <sys/wait.h>

#define DIE(str) perror(str);exit(-1);
#define BUFSIZE 255

// THIS WORKS 
#include <termios.h>
struct termios stdin_orig;

void term_reset(){
	tcsetattr(STDIN_FILENO,TCSANOW,&stdin_orig);
	tcsetattr(STDIN_FILENO,TCSAFLUSH,&stdin_orig);
}

void term_nonblocking(){
	struct termios newt;
	tcgetattr(STDIN_FILENO, &stdin_orig);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	newt = stdin_orig;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	atexit(term_reset);
}
//

int main(int argc, char* argv[]) {
	int portnumber = 1337; // default port
	struct sockaddr_in sin, pin;
	int sd;
	int addrlen;
	char buf[BUFSIZE];
	char* noCommand = "No such command. Connect and try again.\n";
	int pid;
	char message[255];
	bool isQuit=false;
	bool showTables=false;
	bool showSchema=false;

	int new;
	term_nonblocking();
	

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
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		DIE("socket");
	}

	int option = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        /* zero out the sockaddr_in struct */
	memset(&sin, 0, sizeof(sin));
        /* setup the struct to inform the operating system that we would like
         * to bind the socket the the given port number on any network
         * interface using IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(portnumber);

        /* perform bind call */
	if(bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1){
		DIE("bind");
	}
	
	listen(sd, 10); // there wont be any connections waiting since we fork each new client. keep at 10.
	//shutdown(sd, SHUT_RDWR);
	addrlen = sizeof(pin);

	//int status = 1337;

	while(1){
		int test = getchar();
		if(test>0) {
			puts("exit");
			exit(0);
		}
		// accept connection
		new = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen);
		
		// clean the buffers, strange chars will appear in the server console otherwise.
		memset(buf, 0, sizeof buf);
		memset(message, 0, sizeof message);

		if((pid = fork()) == -1){ // something went very wrong
			close(new);
			close(sd);
			continue;
		}else if(pid > 0){  // in parent process (when the child exits)	
			puts("IN PARENT");
			// TODO: fix this :)
			/*if(waitpid(pid, &status, WNOHANG) != -1){
				printf("Process %i terminated sucessfully\n", pid);
			}else{
				puts("Uh-Oh, couldn't terminate process..\n");
			}*/
			close(new);
			continue;
		}else if(pid == 0){ // in child process
			puts("IN CHILD");
			while(1){
				do {					        
					// receive at most sizeof(buf) many bytes and store them in the buffer */
					if(recv(new, buf, sizeof(buf), 0) == -1) { 
						DIE("recv");
					}

					strcat(message, buf);

					// check if the buffer contains the different commands that will have to be executed instantly ( no need for ; )
					isQuit = strstr(buf, ".quit")	    ? true : false;
					showTables = strstr(buf, ".tables") ? true : false;
					showSchema = strstr(buf, ".schema") ? true : false;
					// there was another memset here previosly, doesn't seem to be needed anymore
				}while(!strstr(message, ";") && (!isQuit) && (!showTables) && (!showSchema));
				
				/* convert IP address of communication partner to string */
				char ipAddress[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));
				
				// print the message to server terminal
				printf("%s:%i - %s\n", ipAddress, ntohs(pin.sin_port), message);

				// create the request and parse it
				request_t *request;
				request = parse_request(message);
				if(!request) send(new, noCommand, strlen(noCommand) + 1, 0);				

				if(isQuit){
					printf("Closing connection with %s:%i by request from client.\n", ipAddress, ntohs(pin.sin_port));
					destroy_request(request);
					shutdown(new, SHUT_RDWR); // shutdoooown
					close(new);
					exit(pid);

				}else if(showTables){
					char* allTables = all_tables();
					if(strcmp(allTables, "empty") != 0) {
						send(new, "Showing all tables\n", strlen("Showing all tables\n") + 1, 0);
						send(new, allTables, strlen(allTables) + 1, 0);
					} else {
						send(new, "No tables exists\n", strlen("No tables exists\n") + 1, 0);
					}
					free(allTables);
				}else if(showSchema){
					char* returnSchema = table_schema(request);
					if(strcmp(returnSchema, "Table does not exist") != 0) {					
						send(new, "Showing schema for table\n", strlen("Showing schema for table\n") + 1, 0);
						send(new, returnSchema, strlen(returnSchema) + 1, 0);
					} else {
						send(new, "No such table exists\n", strlen("No such table exists\n") +1, 0);
					}
					free(returnSchema);
				}else{

					char* returnVal;
					switch(request->request_type){
						case RT_CREATE:
							returnVal = create_table(request);
							send(new, returnVal, strlen(returnVal)+1, 0);
							break;
						case RT_DROP:
							returnVal = drop_table(request);
							send(new, returnVal, strlen(returnVal)+1, 0);
							free(returnVal);
							break;
						case RT_INSERT:
							returnVal = insert(request);
							send(new, returnVal, strlen(returnVal)+1, 0);
							break;
						case RT_SELECT:
							returnVal = select_values(request);
							send(new, returnVal, strlen(returnVal)+1, 0);
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
	/* The close for sd never happens, this should be fixed to avoid leaks or open ports etc. */ 
        /* close the file descriptors 
         * NOTE: shutdown() might be a better alternative */
	puts("SHUTDOWN");
	
	shutdown(sd, SHUT_RDWR);
	close(sd);
	exit(0);
}

