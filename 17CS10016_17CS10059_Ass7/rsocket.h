/*
17CS10016 - Jyoti Agrawal
17CS10059 - Koustav Chowdhury 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <signal.h>
#include <netdb.h>

#define MAX_BUFF 100
#define T 2
#define p 0.10
#define SOCK_MRP 135

int r_socket(int domain, int type, int protocol);

int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int r_sendto(int sockfd, const void *buf,size_t len, int flags, struct sockaddr_in *dest_addr, socklen_t addrlen);

int r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t addrlen);

int r_close(int fd);

int dropMessage(float pr);
