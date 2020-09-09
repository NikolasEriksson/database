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
	char completeMessage[255];
	//char createTable[255] = "Successfully created table";

	while(1){
		// accept connection
		//printf(" - new client connected!\n");
		int new = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen);
		// clean the buffer, strange chars will appear in the server console otherwise.
		fflush(stdout);
		int i = 0;				
		for(i = 0; i < sizeof(buf)/sizeof(char); i++) buf[i] = '\0';				
		for(i = 0; i < sizeof(message)/sizeof(char); i++) message[i] = '\0';
		for(i = 0; i < sizeof(completeMessage)/sizeof(char); i++) completeMessage[i] = '\0';
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
				for(i = 0; i < sizeof(completeMessage)/sizeof(char); i++) completeMessage[i] = '\0';		
				

				//send(new, createTable, strlen(createTable)+1, 0);

					/*if(strstr(buf, ".quit") != NULL){
						//send(new, bye, strlen(bye) + 1, 0);
						close(new);
						break;
					}*/
			}
		}
	}
	printf("after loop");
        /* start listening for connections arriving on the bound socket 
         * here, we set the backlog of pending connections to 10*/
//	if(listen(sd, 10) == -1) {
//		DIE("listen");
//	}

//	addrlen = sizeof(pin);
        /* wait for incomming connections;
         * the address information of the communication partner is placed in
         * the provided sockaddr_in struct */
//	if ((sd_current = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen)) == -1) {
//		DIE("accept");
//	}
//	printf("accepted connection\n");
        /* receive at most sizeof(buf) many bytes and store them in the buffer */
//	if (recv(sd_current, buf, sizeof(buf), 0) == -1) {
//		DIE("recv");
//	}

//	char ipAddress[INET_ADDRSTRLEN];
	
        /* convert IP address of communication partner to string */
//	inet_ntop(AF_INET, &pin.sin_addr, ipAddress, sizeof(ipAddress));
	
        /* print out client information and received message */
//	printf("Request from %s:%i\n", ipAddress, ntohs(pin.sin_port));
//	printf("Message: %s\n", buf);
//	printf("\nSend Response:\n");

        /* read reply from standard input */
//	fgets(buf, BUFSIZE - 1, stdin);
        /* determine the length of the response string and send that many bytes
         * from the response buffer to the client*/
//	if(send(sd_current, buf, strlen(buf) + 1, 0) == -1) {
//		DIE("send");
//	}

        /* close the file descriptors 
         * NOTE: shutdown() might be a better alternative */
//	close(sd_current);
//	close(sd);
	exit(0);


}

