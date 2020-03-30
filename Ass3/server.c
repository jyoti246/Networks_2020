#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include<fcntl.h>
#define MAX 100 
#define PORT 8080 
#define SA struct sockaddr 

// Function designed for chat between client and server. 

// Driver function 
int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server accept the client...\n"); 

	// Function for chatting between client and server 
	//func(); 

	char buff[MAX]; 
	int i; 
	bzero(buff, MAX); 

	// read the message from client and copy it in buffer 
	read(connfd, buff, sizeof(buff)); 
	int fd = open(buff, O_RDONLY );
	printf("Requested %s \n", buff);
	if(fd < 0){printf("Cant find the given file.\n");close(sockfd); return ;}
	// infinite loop for chat 
	int x=10;
	while(1) { 
		bzero(buff, MAX); 
		int y=read(fd,buff,sizeof(buff));
		if(y<=0){
			break;
		}
		x=write(connfd, buff, y);
		if(x==-1){close(sockfd); return 0;}
		
	} 
	printf("File sent successfully\n");
	close(fd);
	// After chatting close the socket 
	close(sockfd); 
} 
