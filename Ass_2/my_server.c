#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAX_BUFFER 1024

int main(){
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0){
		perror("Socket creation failed!\n");
		return 0;
	}
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	memset(&cli_addr, 0, sizeof(cli_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8181);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Socket binding failed!\n");
		return 0;
	}
	printf("Server Running ....\n");
	FILE *fptr;
	char buffer[MAX_BUFFER];
	int n;
	socklen_t len;
	len = sizeof(cli_addr);
	n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER, 0, ( struct sockaddr *) &cli_addr, &len);
	buffer[n] = '\0';
	printf("!! File name received is : %s\n", buffer);
	printf("the len is %d\n", n);
	fptr = fopen(buffer, "r");
	if (fptr == NULL){
		char *file_not_found = "FILE NOT FOUND";
		sendto(sockfd, (const char *)file_not_found, strlen(file_not_found), 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
		printf("File Not Found :(\n");
	}
	else{
		char input[MAX_BUFFER];
		char *ending = "END";
		while(fscanf(fptr, "%s", input)){
			sendto(sockfd, (const char *)input, strlen(input), 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
			if (!strcmp(input, ending))
				break;
			len = sizeof(cli_addr);
			n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER, 0, ( struct sockaddr *) &cli_addr, &len);
			buffer[n] = '\0';
			printf("The word received is : %s\n", buffer);
			printf("the len is %d\n", n);
		}
		printf("My job is over :)\n");
	}
	close(sockfd);
	return 0;
}