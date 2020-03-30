#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include<fcntl.h>
#define MAX 10 
#define PORT 8080 
#define SA struct sockaddr 


int main() 
{ 
	int sock, conn; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sock = socket(AF_INET, SOCK_STREAM, 0); 
	if (sock == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sock, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 


	printf("Enter file name you want to get : ");
	// function for chat 
	char buffer[MAX]; 
	scanf("%s",buffer);
	int i=0; 
	write(sock, buffer, sizeof(buffer)); 
	// bzero(buffer, sizeof(buffer)); 
	// read(sock, buffer, sizeof(buffer)); 
	// printf("From Server : %s", buffer); 
	int x=10;
	int fd = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | 007 );
	int characters=0,words=1;
	int flag=0;
	int last =0;
	while(1) { 
		bzero(buffer, sizeof(buffer)); 
		x=read(sock, buffer, sizeof(buffer)); 
		
		if(x!=0){
			flag=1;
			printf("%s",buffer);
			write(fd,buffer,strlen(buffer));
			characters+=strlen(buffer);
			for(int i=0;i<strlen(buffer);i++){
				if(buffer[i]=='.'|buffer[i]==','|buffer[i]==':'|buffer[i]==';'|buffer[i]==' '|buffer[i]=='\n'|buffer[i]=='\t'|buffer[i]=='\v'){
					last=1;
				}else{
					if(last==1)words++;
					last =0;
				}
			}

		}else break;
		
		
	} 
	//printf("%d %d",sock,conn);
	if(flag==0){ printf("“File Not Found\n");close(sock); return 0;}
	if(flag==1){
			close(fd);
			printf("File received successfully in output.txt .Size of the file = %d bytes, no. of words = %d\n",characters,words);
	}
	// close the socket 
	close(fd);
	close(sock); 
} 
