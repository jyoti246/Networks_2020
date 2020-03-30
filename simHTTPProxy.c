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

char    *method,    // "GET" or "POST"
        *uri,       // "/index.html" things before '?'
        *qs,        // "a=1&b=2"     things after  '?'
        *prot;      // "HTTP/1.1"

char    *payload;     // for POST
int      payload_size;


typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = { {"\0", "\0"} };

char *request_header(const char* name);

char *request_header(const char* name)
{
    header_t *h = reqhdr;
    while(h->name) {
        if (strcmp(h->name, name) == 0) return h->value;
        h++;
    }
    return NULL;
}

#define MAXCON 100
#define BUFF_SIZE 4096

void service_client(int cfd, int sfd)
{
    int maxfd;
    char *sbuf;
    char *cbuf;
    cbuf = malloc(BUFF_SIZE);
    int x, y;
    x = read(cfd, cbuf, sizeof(cbuf));
    if(x<0){
        close(cfd);
        close(sfd);
        printf("A connnection closed\n\n");
        free(cbuf);
        return;
    }
    
    y = send(sfd, cbuf, x, 0);
    if(y<0){
        close(cfd);
        close(sfd);
        printf("A connnection closed\n\n");
    }
    
    free(cbuf);
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
char *buf;
int mywrite(int fd, char *buf1, int *len)
{
    int x = write(fd, buf1, *len);
    if (x < 0)
        return x;
    if (x == 0)
        return x;
    if (x != *len)
        memmove(buf1, buf1+x, (*len)-x);
    *len -= x;
    return x;
}

int respond(int sockfd)
{
    int rcvd, fd, bytes_read;
    char *ptr;

    int sockfd_in_ = sockfd;

    buf = malloc(65535);
    rcvd = recv(sockfd, buf, 65535, 0);

    if (rcvd<0){    // receive error
        fprintf(stderr,("recv() error\n"));
        return -1;
    }
    else if (rcvd==0){    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
        return -1;
    }
    else    // message received
    {
        buf[rcvd] = '\0';
        char* tempstr = calloc(strlen(buf)+1, sizeof(char));
        strcpy(tempstr, buf);
        // printf("%s\n", buf);
        method = strtok(buf,  " \t\r\n");

        // printf("\n::::\n%s\n", buf);
        uri    = strtok(NULL, " \t");
        prot   = strtok(NULL, " \t\r\n"); 

        fprintf(stdout, "The method is %s\n", method);
        fprintf(stdout, "The uri is %s\n", uri);
        fprintf(stdout, "The protocol is %s\n", prot);

        int get_or_post = 0;
        char *get_req = "GET";
        char *post_req = "POST";

        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; //split URI
        } else {
            qs = uri - 1; //use an empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;
        char *host_ = "Host";
        char *hostname_;
        while(h < reqhdr+16) {
            char *k,*v,*t;
            k = strtok(NULL, "\r\n: \t"); if (!k) break;
            v = strtok(NULL, "\r\n");     while(*v && *v==' ') v++;
            h->name  = k;
            h->value = v;
            if (!strcmp(h->name, host_)){
                hostname_ = h->value;
            }
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n') break;
        }
        t++; // now the *t shall be the beginning of user payload
        t2 = request_header("Content-Length"); // and the related header if there is  
        payload = t;
        payload_size = t2 ? atol(t2) : (rcvd-(t-buf));


        if (!strcmp(method, get_req) || !strcmp(method, post_req))
            get_or_post = 1;

        if (!get_or_post)
            return -1;

        if (get_or_post){ // GET REQUEST
            struct hostent *lh = gethostbyname(hostname_);
            fprintf(stderr, "%s\n", hostname_);
            const char *ip_ = (const char *) inet_ntoa( (struct in_addr) *((struct in_addr *) lh->h_addr_list[0]));
            printf("ip is %s\n", ip_);
            struct sockaddr_in remoteaddr;

            remoteaddr.sin_family = AF_INET;
            remoteaddr.sin_port = htons(80);
            if (inet_pton(AF_INET, ip_, &remoteaddr.sin_addr) < 0){
                fprintf(stderr, "invalid proxy\n");
                return -1;
                exit(4);
            }
            int sockfd_out_;
            if ((sockfd_out_ = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                fprintf(stderr, "socket failed: %s\n", strerror(errno));
                return -1;
                exit(5);
            }
            // printf("I am here!\n");
            set_nonblock(sockfd_out_);  
            int contchk=connect(sockfd_out_, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
            if(contchk<0){
                printf("\nWaiting to connect \n");
                // return -1;
                // exit(5);
            }
            // printf("i/p sock = %d, o/p sock = %d\n", sockfd, sockfd_out_);

            int bsent = 0;
            int y;
            // printf("received : %d\n", rcvd);
            int tosend = rcvd;
            char *cl_buf = tempstr;
            // printf("%s\n", cl_buf);
            while (tosend > 0){
                // printf("Loop k andar\n");
                y = mywrite(sockfd_out_, cl_buf, &tosend);
                // printf("y = %d\n", y);
                bsent += y;
                // printf("bsent : %d , tosend : %d\n", bsent, tosend);
            }
            contchk=connect(sockfd_out_, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
            if(contchk>=0){
                printf("\nwaiting for reply ...\n");
                // return -1;
                // exit(5);
            }else{
                printf("\nNot yet connected\n");
            }
            free(buf);
            free(tempstr);
            return sockfd_out_;
        }
    }
}

int main(int argc, char *argv[]){
    int i, x, y;
    if (argc != 2){
            fprintf(stderr, "Correct usage:\n./simHTTPProxy <listen port>\n");
            return 0;
    }
    int localport = atoi(argv[1]);
    memset(&inpaddr, '\0', sizeof(inpaddr));
    
    inpaddr.sin_family = AF_INET;
    inpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inpaddr.sin_port = htons(atoi(argv[1]));

    int opt = 1;

    if ((mast_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            fprintf(stderr,"socket creation failed\n");
            exit(1);
    }
   	set_nonblock(mast_sock);
    if(setsockopt(mast_sock, SOL_SOCKET, SO_REUSEADDR, &opt, 4)){
        perror("Setsockopt failed for socket \n");
        exit(1);
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
    int cnt = 0;
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
        if(isready<0){
            for (i = 0 ; i < idx; i++){
                close(sockfd_in[i]);
                close(sockfd_out[i]);
            }
            close(mast_sock);
            printf("ERROR in select\n\n");
        }
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
                if(idx >= MAXCON )printf("\n\n\nNo new connections allowed now\n\n\n");
                if (idx < MAXCON && ((sockfd_in[idx] = accept(mast_sock, (struct sockaddr *)&cliaddr, &len_cli))) >= 0){
                    char info[100];
                    inet_ntop(AF_INET, &(cliaddr.sin_addr), info, 100);
                    
                    printf("> Connection accepted from %s : %d\n", info, (int)ntohs(cliaddr.sin_port));
                    int remote_socket = respond(sockfd_in[idx]);
                    if (remote_socket < 0)
                        continue;
                    else{
                        sockfd_out[idx] = remote_socket;
                    }
                    idx++;
                }     
            } 
            for (i = 0; i < idx; i++)
            {
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