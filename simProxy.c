#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <syslog.h>
#include <err.h>


#include <sys/types.h>
#include <sys/select.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

// Source of the following piece of code : https://github.com/wessels/simple-tcp-proxy/blob/master/simple-tcp-proxy.c

// // #########################################################################################################

// typedef struct info{
//     int cfd, sfd;
// } info;

// int mywrite(int fd, char *buf, int *len)
// {
//     int x = write(fd, buf, *len);
//     if (x < 0)
//         return x;
//     if (x == 0)
//         return x;
//     if (x != *len)
//         memmove(buf, buf+x, (*len)-x);
//     *len -= x;
//     return x;
// }

// void *service_client(void *args)
// {
//     info *cfd_sfd = (info *)args;
//     int cfd = cfd_sfd->cfd;
//     int sfd = cfd_sfd->sfd;
//     int maxfd;
//     char *sbuf = (char *)malloc(BUF_SIZE);
//     char *cbuf = (char *)malloc(BUF_SIZE);
//     int x, n;
//     int cbo = 0;
//     int sbo = 0;
//     fd_set R;

//     maxfd = max(cfd, sfd);
//     maxfd++;

//     for (;;) {
//     struct timeval to;
//     if (cbo) {
//         if (mywrite(sfd, cbuf, &cbo) < 0 && errno != EWOULDBLOCK) {
//             fprintf(stderr, "write %d: %s", sfd, strerror(errno));
//             exit(1);
//         }
//     }
//     if (sbo) {
//         if (mywrite(cfd, sbuf, &sbo) < 0 && errno != EWOULDBLOCK) {
//             fprintf(stderr, "write %d: %s", cfd, strerror(errno));
//             exit(1);
//         }
//     }
//     FD_ZERO(&R);
//     if (cbo < BUF_SIZE)
//         FD_SET(cfd, &R);
//     if (sbo < BUF_SIZE)
//         FD_SET(sfd, &R);
//     to.tv_sec = 0;
//     to.tv_usec = 1000;
//     x = select(maxfd+1, &R, 0, 0, &to);
//     if (x > 0) {
//         if (FD_ISSET(cfd, &R)) {
//         n = read(cfd, cbuf+cbo, BUF_SIZE-cbo);
//         fprintf(stdout, "read %d bytes from CLIENT (%d)", n, cfd);
//         if (n > 0) {
//             cbo += n;
//         } else {
//             close(cfd);
//             close(sfd);
//             fprintf(stdout, "exiting ...");
//             _exit(0);
//         }
//         }
//         if (FD_ISSET(sfd, &R)) {
//         n = read(sfd, sbuf+sbo, BUF_SIZE-sbo);
//         fprintf(stdout, "read %d bytes from SERVER (%d)", n, sfd);
//         if (n > 0) {
//             sbo += n;
//         } else {
//             close(sfd);
//             close(cfd);
//             fprintf(stdout, "exiting");
//             _exit(0);
//         }
//         }
//     } else if (x < 0 && errno != EINTR) {
//         fprintf(stderr, "select: %s", strerror(errno));
//         close(sfd);
//         close(cfd);
//         fprintf(stdout, "exiting");
//         _exit(0);
//     }
//     }
// }

// int open_remote_host(char *host, int port)
// {
//     struct sockaddr_in rem_addr;
//     int len, s, x;
//     struct hostent *H;
//     int on = 1;

//     H = gethostbyname(host);
//     if (!H)
//     return (-2);

//     len = sizeof(rem_addr);

//     s = socket(AF_INET, SOCK_STREAM, 0);
//     if (s < 0)
//     return s;

//     setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, 4);

//     len = sizeof(rem_addr);
//     memset(&rem_addr, '\0', len);
//     rem_addr.sin_family = AF_INET;
//     memcpy(&rem_addr.sin_addr, H->h_addr, H->h_length);
//     rem_addr.sin_port = htons(port);
//     x = connect(s, (struct sockaddr *) &rem_addr, len);
//     if (x < 0) {
//     close(s);
//     return x;
//     }
//     set_nonblock(s);
//     return s;
// }

// // #######################################################################



#define MAXCON 5000
#define BUFF_SIZE 4096



void service_client(int cfd, int sfd)
{
    int maxfd;
    char *sbuf;
    char *cbuf;
    int cbo = 0;
    int sbo = 0;
    fd_set R, W;

    sbuf = malloc(BUFF_SIZE);
    cbuf = malloc(BUFF_SIZE);
    maxfd = cfd > sfd ? cfd : sfd;
    maxfd++;

    FD_ZERO(&R);
    FD_ZERO(&W);

    FD_SET(cfd, &R);
    FD_SET(sfd, &W);

    int x, y;
    x = read(cfd, cbuf, sizeof(cbuf));
    y = send(sfd, cbuf, x, 0);
}


struct sockaddr_in proxyaddr, inpaddr, cliaddr;
socklen_t len, len_cli;

int mast_sock, maxfd, isready;

void set_nonblock(int fd)
{
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) {
        fprintf(stderr, "fcntl F_GETFL: FD %d: %s", fd, strerror(errno));
        exit(2);
    }
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
        fprintf(stderr, "fcntl F_SETFL: FD %d: %s", fd, strerror(errno));
        exit(3);
    }
}

int max(int a, int b)
{
        return a > b ? a : b;
}


int sockfd_out[MAXCON], sockfd_in[MAXCON];
fd_set incoming, outgoing;

int main(int argc, char *argv[]){
    int i, x, y;
    signal(SIGPIPE, SIG_IGN);
    if (argc != 4){
            fprintf(stderr, "Correct usage:\n./simProxy <listen port> <institute_proxy> <institute_proxy_port>\n");
            return 0;
    }
    int localport = atoi(argv[1]);
    char *proxy_server = strdup(argv[2]);
    char remoteport = atoi(argv[3]);

    memset(&inpaddr, '\0', sizeof(inpaddr));
    memset(&proxyaddr, '\0', sizeof(proxyaddr));

    inpaddr.sin_family = AF_INET;
    inpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inpaddr.sin_port = htons(atoi(argv[1]));

    proxyaddr.sin_family = AF_INET;
    proxyaddr.sin_port = htons(atoi(argv[3]));

    int opt = 1;

    if ((mast_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            fprintf(stderr,"socket creation failed\n");
            exit(1);
    }
    set_nonblock(mast_sock);
    setsockopt(mast_sock, SOL_SOCKET, SO_REUSEADDR, &opt, 4);

    if (inet_pton(AF_INET, argv[2], &proxyaddr.sin_addr) < 0){
        fprintf(stderr, "invalid proxy\n");
        exit(4);
    }

    if (bind(mast_sock, (struct sockaddr *)&inpaddr, sizeof(inpaddr)) < 0){
        fprintf(stderr, "bind failure mast_sock\n");
        exit(5);
    }

    if (listen(mast_sock, MAXCON) < 0){
        fprintf(stderr, "sunai nhi de rha\n");
        exit(6);
    }

    FD_ZERO(&incoming);
    FD_ZERO(&outgoing);

    int idx = 0;

    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 1;

    for (;;){

        FD_ZERO(&incoming);
        FD_ZERO(&outgoing);
        FD_SET(0, &incoming); // stdin added to incoming buffer
        maxfd = 0;
        FD_SET(mast_sock, &incoming);

        maxfd = max(maxfd, mast_sock);
        
        for (i = 0; i < idx; i++){
            FD_SET(sockfd_in[i], &incoming);
            FD_SET(sockfd_out[i], &incoming);
            FD_SET(sockfd_in[i], &outgoing);
            FD_SET(sockfd_out[i], &outgoing);
            // printf("i = %d : in %d out %d\n",i, sockfd_in[i], sockfd_out[i]);
            maxfd = max(maxfd, sockfd_in[i]);
            maxfd = max(maxfd, sockfd_out[i]);
        }

        isready = select(maxfd + 1, &incoming, &outgoing, NULL, &to);
        if (isready > 0){
            if (FD_ISSET(0, &incoming)){
                char buf[256];
                scanf("%s", buf);
                const char *ex = "exit";
                if (!strcmp(buf, ex)){
                    for (i = 0 ; i < idx; i++){
                        close(sockfd_in[i]);
                        close(sockfd_out[i]);
                    }
                    close(mast_sock);
                    return 0;
                } 
            }
            else if (FD_ISSET(mast_sock, &incoming)){
                if (idx < MAXCON && ((sockfd_in[idx] = accept(mast_sock, (struct sockaddr *)&cliaddr, &len_cli))) >= 0){
                    char info[100];
                    inet_ntop(AF_INET, &(cliaddr.sin_addr), info, 100);
                    
                    printf("> Connection accepted from %s : %d\n", info, (int)ntohs(cliaddr.sin_port));
                    
                    if ((sockfd_out[idx] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                        fprintf(stderr, "socket failed: %s\n", strerror(errno));
                        exit(0);
                    }
                    set_nonblock(sockfd_out[idx]);  
                    connect(sockfd_out[idx], (struct sockaddr *)&proxyaddr, sizeof(proxyaddr));
                    idx++;
                }     
            } 
            for (i = 0; i < idx; i++)
            {
                char tbuffer[BUFF_SIZE];
                if (FD_ISSET(sockfd_in[i], &incoming) && FD_ISSET(sockfd_out[i], &outgoing)){
                    service_client(sockfd_in[i], sockfd_out[i]);
                }
                if (FD_ISSET(sockfd_out[i], &incoming) && FD_ISSET(sockfd_in[i], &outgoing)){
                    service_client(sockfd_out[i], sockfd_in[i]);
                }
            }
        }
    }
    return 0;
}