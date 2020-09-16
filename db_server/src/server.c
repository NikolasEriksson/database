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

#define DIE(str) perror(str);exit(-1);
#define BUFSIZE 255


int main(int argc, char* argv[]) {
	int portnumber = 1337; // default port
	struct sockaddr_in sin, pin;
	int sd;//, sd_current;
	int addrlen;
	char buf[BUFSIZE];
	char* noCommand = "No such command. Connect and try again.\n";
	int pid;
	char message[255];
	bool isQuit=false;
	bool showTables=false;
	bool showSchema=false;
	int numberOfConnections=0;
	int new;

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
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		DIE("socket");
	}

        /* zero out the sockaddr_in struct */
	memset(&sin, 0, sizeof(sin));
        /* setup the struct to inform the operating system that we would like
         * to bind the socket the the given port number on any network
         * interface using IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(portnumber);

        /* perform bind call */
	if(bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
		DIE("bind");
	}
	listen(sd, 2);
	//shutdown(sd, SHUT_RDWR);
	addrlen = sizeof(pin);

	while(1){
		// accept connection

		new = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen);

		// clean the buffer, strange chars will appear in the server console otherwise.
		fflush(stdout);
		int i = 0;				
		for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';				
		for(i = 0; i < sizeof(message)/sizeof(char); i++) message[i] = '\0';
		if((pid = fork()) == -1){
			close(new);
			close(sd);
			continue;
		}else if(pid > 0){
			close(new);
			continue;
		}else if(pid == 0){	
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
					int i = 0;
					for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0'; // clear the buffer
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
					numberOfConnections-=1;
					destroy_request(request);
					shutdown(new, SHUT_RDWR); // shutdoooown
					break;
				}else if(showTables){
					send(new, "showing all tables\n", strlen("showing all tables\n") + 1, 0);
					char* allTables = all_tables();
					send(new, allTables, strlen(allTables) + 1, 0);
					//printf("%s", allTables);			
					//print_request(request);
			
				}else if(showSchema){
					send(new, "showing schema\n", strlen("showing schema\n") + 1, 0);
					print_request(request);
				}else{

					char* returnVal;
					if(request->request_type == 0) returnVal = create_table(request);
					send(new, returnVal, strlen(returnVal)+1, 0);
					//if(request->request_type == 4) INSERT(request);


				}
				// DESTROY the request
				destroy_request(request);
				// clean all buffers, strange chars will appear in the server console otherwise.
				fflush(stdout);
				fflush(stdin);
				int i = 0;
				for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';
				for(i = 0; i < sizeof(message)/sizeof(char); i++) message[i] = '\0';
			}
		}
	}
	/* The close for sd never happens, this should be fixed to avoid leaks or open ports etc. */ 
        /* close the file descriptors 
         * NOTE: shutdown() might be a better alternative */
	close(sd);
	exit(0);
}

