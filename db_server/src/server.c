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
				do {
					//int i = 0;
					//for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';				
					if(recv(new, buf, sizeof(buf), 0) == -1) {
						DIE("recv");
					}
					strcat(message, buf);
					int i = 0;
					for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';
				}while((!strstr(message, ";")) || // ta hand om den här jävka skiten (1<2 ? printf("quit!!!") : printf("dont!")));
				
	


				if(strstr(message, ".quit") != NULL){
					close(new);
					break;	
				}
				
				char ipAddress[INET_ADDRSTRLEN];

				/* convert IP address of communication partner to string */
				inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));

				printf("%s:%i - %s\n", ipAddress, ntohs(pin.sin_port), message);


				// clean the buffer, strange chars will appear in the server console otherwise.
				fflush(stdout);
				fflush(stdin);
				int i = 0;
				for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';
				for(i = 0; i < sizeof(message)/sizeof(char); i++) message[i] = '\0';			
				

				//send(new, createTable, strlen(createTable)+1, 0);

					/*if(strstr(buf, ".quit") != NULL){
						//send(new, bye, strlen(bye) + 1, 0);
						close(new);
						break;
					}*/
			}
		}
	}

        /* close the file descriptors 
         * NOTE: shutdown() might be a better alternative */
	close(sd_current);
	close(sd);
	exit(0);
}

