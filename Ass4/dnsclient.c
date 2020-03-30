#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAX_BUFFER 100

int main(){
	int sockfd;
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0){
		perror("Socket creation failed!!\n");
		return 0;
	}
	char host_name[MAX_BUFFER] ;
	printf("Enter host name : ");
	scanf("%s",host_name);
	// printf("hdhdcedc");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	char buffer[MAX_BUFFER];
	printf("Host name sent : %s\n", host_name);
	sendto(sockfd, (const char *)host_name, strlen(host_name), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
	int n;
	socklen_t len;
	len = sizeof(serv_addr);
	n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER, 0, (struct sockaddr *)&serv_addr, &len);
	buffer[n] = '\0';
	printf("IP of this Host name : %s\n",buffer);
	close(sockfd);
	return 0;
}