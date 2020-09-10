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

#define DIE(str) perror(str);exit(-1);
#define BUFSIZE 255


int main(int argc, char* argv[]) {
	int portnumber;
	struct sockaddr_in sin, pin;
	int sd, sd_current;
	int addrlen;
	char buf[BUFSIZE];

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(-1);
	}

	portnumber = atoi(argv[1]);

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

	listen(sd, 10);
	addrlen = sizeof(pin);
	int pid;
	char message[255];
	bool isQuit=false;
	bool showTables=false;
	bool showSchema=false;
	while(1){
		// accept connection
		int new = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen);
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
			        // receive at most sizeof(buf) many bytes and store them in the buffer */
				int test=0;
				do {			
					if(recv(new, buf, sizeof(buf), 0) == -1) {
						DIE("recv");
					}
					strcat(message, buf);
					// check if the buffer contains the different commands that will have to be executed instantly
					isQuit = strstr(buf, ".quit") 	    ? true : false;
					showTables = strstr(buf, ".tables") ? true : false;
					showSchema = strstr(buf, ".schema") ? true : false;
					int i = 0;
					for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';
				}while(!strstr(message, ";") && (!isQuit) && (!showTables) && (!showSchema));
				
				/*  should use strcmp for these statements, strstr checks substring, i.e. .tables123 = valid  */ 

				// if the buffer (and message) contained ".quit"
				if(strstr(message, ".quit") != NULL){
					close(new);
					break;	
				}
				// if the buffer (and message) contained ".tables"
				if(strstr(message, ".tables") != NULL){
					//printf("showing all tables");
					send(new, "showing all tables\n", strlen("showing all tables\n") + 1, 0);
				}
				// if the buffer (and message) contained ".schema"
				if(strstr(message, ".schema") != NULL){
					send(new, "showing all schemas\n", strlen("showing all schemas\n") + 1, 0);
				}
				char ipAddress[INET_ADDRSTRLEN];

				/* convert IP address of communication partner to string */
				inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));

				printf("%s:%i - %s\n", ipAddress, ntohs(pin.sin_port), message);


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

