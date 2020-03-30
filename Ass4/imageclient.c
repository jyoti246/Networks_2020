#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include<fcntl.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>    
#include<unistd.h>
#include<errno.h>
#define MAX 100 
#define PORT 8080 
#define SA struct sockaddr 


int main() 
{ 
	int sock, conn; 
	struct sockaddr_in servaddr, cli; 
	int count=0;
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
	// printf("%s",buffer);

	bzero(buffer, sizeof(buffer)); 
	while(1){
		int buffersize = 0, recv_size = 0,size = 0, read_size, write_size;
		char verify = '1';
		FILE *image;
	
		//Find the size of the image
		read(sock,buffer,sizeof(buffer));
		// printf("%s",buffer);
		if(buffer[0]=='E')break;
		bzero(buffer, sizeof(buffer)); 
		read(sock, &size, sizeof(int));
		
		// printf("%d\n",size);
	
		//Send our verification signal
		write(sock, &verify, sizeof(char));
		//Make sure that the size is bigger than 0

		if(size <= 0 ){
		    printf("Error has occurred. Size less than or equal to 0\n");
		    return -1;
		}
		read(sock, buffer, sizeof(buffer));
		// printf("%s\n",buffer);
		image = fopen(buffer, "w+");
		bzero(buffer, sizeof(buffer)); 
		if( image == NULL) {
		    printf("Error has occurred. Image file could not be opened\n");
		    return -1;
		}
	
		//Loop while we have not received the entire file yet
		while(recv_size < size) {
		    ioctl(sock, FIONREAD, &buffersize);
		    if(count<30) 
			// printf("rec= %d size= %d \n",recv_size,size);
			count++;
		    //We check to see if there is data to be read from the socket    
		    if(buffersize > 0 ) {
	
		        if((read_size = read(sock,buffer, buffersize)) < 0){
		            printf("%s\n", strerror(errno));
		        }
				// printf("bufsiz = %d",buffersize);
		        // printf("read size = %d, write size = %d\n", read_size, write_size );
		        //Write the currently read data into our image file
		        write_size = fwrite(buffer,1,(buffersize), image);
	
		        if(write_size != buffersize) {
		          printf("write and buffersizes wrong\n");
		        }
	
		        if(read_size !=write_size) {
		            // printf("error in read write\n");
		        }
	
		        //Increment the total number of bytes read
		        recv_size += read_size;
				bzero(buffer, sizeof(buffer)); 
		                    //Send our handshake verification info
		        write(sock, &verify, sizeof(char));
	
		    }
		}
	
		fclose(image);
		printf("Image successfully Received!\n");
		// read(sock, buffer, sizeof(buffer)); 
	}
	// printf("From Server : %s", buffer); 
	// int x=10;
	// int fd = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | 007 );
	// int characters=0,words=1;
	// int flag=0;
	// int last =0;
	// while(1) { 
	// 	bzero(buffer, sizeof(buffer)); 
	// 	x=read(sock, buffer, sizeof(buffer)); 
		
	// 	if(x!=0){
	// 		flag=1;
	// 		printf("%s",buffer);
	// 		write(fd,buffer,x);
	// 		characters+=strlen(buffer);
	// 		for(int i=0;i<strlen(buffer);i++){
	// 			if(buffer[i]=='.'|buffer[i]==','|buffer[i]==':'|buffer[i]==';'|buffer[i]==' '|buffer[i]=='\n'|buffer[i]=='\t'|buffer[i]=='\v'){
	// 				last=1;
	// 			}else{
	// 				if(last==1)words++;
	// 				last =0;
	// 			}
	// 		}

	// 	}else break;
		
		
	// } 
	// //printf("%d %d",sock,conn);
	// if(flag==0){ printf("“File Not Found\n");close(sock); return 0;}
	// if(flag==1){
	// 		close(fd);
	// 		printf("File received successfully in output.txt .Size of the file = %d bytes, no. of words = %d\n",characters,words);
	// }
	// close the socket 
	// close(fd);
	close(sock); 
} 