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
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0){
		perror("Socket creation failed!!\n");
		return 0;
	}
	char *file_name = "file_1.txt";
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8181);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	char buffer[MAX_BUFFER];
	printf("File name sent : %s\n", file_name);
	sendto(sockfd, (const char *)file_name, strlen(file_name), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
	int n;
	socklen_t len;
	len = sizeof(serv_addr);
	n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER, 0, (struct sockaddr *)&serv_addr, &len);
	buffer[n] = '\0';
	const char *hello = "HELLO";
	const char *ending = "END";
	// printf("%s\n", buffer);
	FILE *fptr;
	if (!strcmp(buffer, hello)){
		char new_file[] = "client_";
		strcat(new_file, file_name);
		printf("New file name is %s\n", new_file);
		fptr = fopen(new_file, "w");
		char number[MAX_BUFFER];
		int num = 1;
		do{
			char word[] = "WORD";
			sprintf(number, "%d", num);
			num++;
			strcat(word, number);
			sendto(sockfd, (const char *)word, strlen(word), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
			len = sizeof(serv_addr);
			n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER, 0, (struct sockaddr *)&serv_addr, &len);
			buffer[n] = '\0';
			printf("The word received is : %s\n", buffer);
			if (!strcmp(buffer, ending)){
				break;
			}
			else{
				buffer[n] = '\n';
				buffer[n+1] = '\0';
				fputs(buffer, fptr);
			}
		}while(1);
		fclose(fptr);
	} 
	else{
		perror("File not found\n");
	}
	close(sockfd);
	return 0;
}