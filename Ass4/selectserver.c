#include <netdb.h> 
#include <netinet/in.h>  
#include <sys/types.h> 
#include<fcntl.h>
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
#include <dirent.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#define MAX 100 
#define PORT 8080 
#define SA struct sockaddr 

int max(int x, int y) 
{ 
    if (x > y) 
        return x; 
    else
        return y; 
} 

// Function designed for chat between cli_addrent and server. 

// Driver function 
int main() 
{ 
    int  udpfd, nready, maxfdp1; 
    int sockfd, connfd, len; 
    struct sockaddr_in serv_addr, cli_addr; 
    char buffer[MAX]; 
    pid_t childpid; 
    fd_set rset; 
    ssize_t n; 
    socklen_t lent; 
    const int on = 1; 
    // socket create and verification 



    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&serv_addr, sizeof(serv_addr)); 

    // assign IP, PORT 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(PORT); 




    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&serv_addr, sizeof(serv_addr))) != 0) { 
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
    // len = sizeof(cli_addr); 





    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0){
        perror("Socket creation failed!\n");
        return 0;
    }
    
    if (bind(udpfd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Socket binding failed!\n");
        return 0;
    }
    printf("Server Running ....\n");



    // clear the descriptor set 
    FD_ZERO(&rset); 
    // get maxfd 
    maxfdp1 = max(sockfd, udpfd) + 1; 




    while(1){
        // printf("\n\n\nLOOP\n\n\n");
        // set listenfd and udpfd in readset 
        FD_SET(sockfd, &rset); 
        FD_SET(udpfd, &rset); 


        // select the ready descriptor 
        nready = select(maxfdp1, &rset, NULL, NULL, NULL);


        // if tcp socket is readable then handle 
        // it by accepting the connection 
        if (FD_ISSET(sockfd, &rset)) { 
            len = sizeof(cli_addr); 
            connfd = accept(sockfd, (struct sockaddr*)&cli_addr, &len); 
            if (connfd < 0) { 
                printf("TCP server acccept failed...\n"); 
                exit(0); 
            } 
            else
                printf("TCP server accept the cli_addrent...\n"); 

            if ((childpid = fork()) == 0) { 
                int i; 
                bzero(buffer, MAX); 

                // read the message from cli_addrent and copy it in buffer 

                int z = read(connfd, buffer, sizeof(buffer)); 
                char loc[100]="image/";
                z = strlen(buffer);
                // printf("loc now = %s, buffer = %s, loc_length = %d, z = %d, buffer_length = %d\n",loc, buffer, strlen(loc), z, strlen(buffer));
                for(i=0;i<z;i++){
                    loc[6+i]=buffer[i];
                }
                loc[z+6]='/';
                loc[z+7]='\0';
                // printf("%s",loc);
                struct dirent *de;
                DIR *dr = opendir(loc); 
                
                if (dr == NULL)  // opendir returns NULL if couldn't open directory 
                { 
                    printf("Cant find the given folder.\n");close(sockfd); 
                    return 0; 
                } 
                
                // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
                // for readdir() 
                while ((de = readdir(dr)) != NULL){
                    if((de->d_name)[0]!='.'){
                        FILE *picture;
                        int size, read_size;
                        char verify;
                        char imloc[100];
                        //imloc = loc;
                        for(i=0;i<50;i++){
                            imloc[i]=loc[i];
                        }
                        // printf("img name= %s",(de->d_name));
                        for(i=0;i<50;i++){
                            imloc[z+7+i]=(de->d_name)[i];
                        }
                        imloc[z+7+strlen(de->d_name)]='\0';
                        printf("imloc= %s",imloc);
                        // p?rintf("");
                        picture = fopen(imloc, "r");
                        printf("Getting Picture Size\n");   

                        if(picture == NULL) {
                           printf("Error Opening Image File");
                        } 

                        fseek(picture, 0, SEEK_END);
                        size = ftell(picture);
                        fseek(picture, 0, SEEK_SET);

                        //Send Picture Size
                        //buffer="";
                        buffer[0]='C';
                        buffer[1]='\0';
                        // printf("%s",buffer);
                        write(connfd,buffer,sizeof(buffer));
                        bzero(buffer, sizeof(buffer));

                        printf("Sending Picture Size = %d\n",size);
                        write(connfd, (void *)&size, sizeof(int));
                        // printf("1");
                        if(read_size = read(connfd, &verify , sizeof(char)) < 0) {
                           // puts("\nError Receiving Verification");
                            printf("\nError Receiving Verification");
                        }
                        // buffer=(de->d_name);
                        write(connfd, (de->d_name), strlen((de->d_name)));
                        // printf("%s",(de->d_name));
                        bzero(buffer, sizeof(buffer));
                        if(verify == '1'){
                            // printf("5\n");
                            // //Send Picture as Byte Array
                            // printf("Sending Picture as Byte Array\n");

                            while(!feof(picture)) {

                                  //Read from the file into our send buffer
                                  read_size = fread(buffer, 1, sizeof(buffer)-1, picture);

                                  //Send data through our socket 
                                  write(connfd, buffer, read_size);                        
                                  // printf("%s",buffer);
                                  //Wait for the verify signal to be received 
                                  while(read(connfd, &verify , sizeof(char)) < 0);

                                  if(verify != '1') {
                                     printf("Error Receiving the Handshake signal\n %s",&verify);
                                  }

                                  verify = ' ';

                                  //Zero out our send buffer
                                  bzero(buffer, sizeof(buffer));
                           }
                        }

                    }
                }
                bzero(buffer, sizeof(buffer));
                //buffer;
                buffer[0]='E';
                buffer[1]='O';
                buffer[2]='F';
                buffer[3]='\0';
                write(connfd, buffer, 4);
                // int fd = open(buff, O_RDONLY );
                // printf("Requested %s \n", buff);
                // if(fd < 0){printf("Cant find the given file.\n");close(sockfd); return ;}
                // // infinite loop for chat 
                // int x=10;
                // while(1) { 
                //  bzero(buff, MAX); 
                //  int y=read(fd,buff,sizeof(buff));
                //  if(y<=0){
                //      break;
                //  }
                //  x=write(connfd, buff, y);
                //  if(x==-1){close(sockfd); return 0;}
                    
                // } 
                printf("Images sent successfully\n");
                //close(fd);
                closedir(dr);
                // After chatting close the socket 
                // close(sockfd); 
                close(connfd); 
                exit(0); 
            } 
            close(connfd); 
        } 
        // if udp socket is readable receive the message. 
        if (FD_ISSET(udpfd, &rset)) { 
            printf("\n\nUDP Server Running ....\n");
            len = sizeof(cli_addr); 
            bzero(buffer, sizeof(buffer)); 
            int n;
            // socklen_t len;
            len = sizeof(cli_addr);

            n = recvfrom(udpfd, (char *)buffer, MAX, 0, ( struct sockaddr *) &cli_addr, &len);
            buffer[n] = '\0';
            printf("!! Host name received is : %s\n", buffer);
            printf("the len is %d\n", n);
            struct hostent *lh = gethostbyname(buffer);
            printf("IP : ");
            if (lh)
                puts(inet_ntoa( (struct in_addr) *((struct in_addr *) lh->h_addr_list[0])));
            else
                herror("gethostbyname");
            // FILE *fptr;
            // fptr = fopen(buffer, "r");
            printf("IP sent successfully....\n\n");
            char *file_not_found = inet_ntoa( (struct in_addr) *((struct in_addr *) lh->h_addr_list[0]));
            sendto(udpfd, (const char *)file_not_found, strlen(file_not_found), 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
            
            

        }
    }


} 