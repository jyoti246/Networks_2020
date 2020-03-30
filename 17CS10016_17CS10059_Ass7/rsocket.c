/*
17CS10016 - Jyoti Agrawal
17CS10059 - Koustav Chowdhury 
*/

#include "rsocket.h"

// keeping the variable global for use of access

int fd; 	// socket file descriptor
int send_count; // no of msg sent
int recv_count;	// no of msg recieved
int unACK_count; // no of un-acknowledged messages
int fla; // flags

// book-keeping variables
int recv_id_count; // count of no of recieved msg
int retrans_count; // count of no of retransmitted  msg

char ACK[]="acknowledgement\00";

struct send_msg
{
	int id;
	char buff[MAX_BUFF];
	struct sockaddr_in addr;
};

struct receive_msg
{
	char buff[MAX_BUFF];
	struct sockaddr_in addr;
};

struct unacknowledged_msg
{
	int id;
	char buff[MAX_BUFF];
	time_t time;
	struct sockaddr_in addr;
};

struct receive_id_msg
{
	int id;
	struct sockaddr_in addr;
};

typedef struct send_msg send_;
typedef struct receive_msg recv_;
typedef struct unacknowledged_msg unACK_;
typedef struct receive_id_msg recv_id_;

send_ *sendBuff;
recv_ *recvBuff;
unACK_ *unACKTable;
recv_id_ *recvIDTable;

int increment(int x){
	return x+1;
}

void Signal_Handler(int signum)
{
	check_if_finished();
	HandleReceive();
	HandleRetransmit();
	HandleTransmit();
}

void init_recv_table(){
	int i = 0;
	while (i<MAX_BUFF){
		recvIDTable[i].id=-1;
		i++;
	}
}

void check_if_finished()
{
	if(!send_count && !unACK_count && !recv_count && !fla)
	{
		fprintf(stdout, "\n:: Count of retransmissions is %d\n", retrans_count);
		r_close(fd);
		exit(0);
	}
}

int dropMessage(float pr)
{
	float x=(float)rand()/(float)(RAND_MAX);
	return (x < pr) ? 1 : 0;
}

int HandleReceive()
{
	char buff[MAX_BUFF];
	memset(buff,0,MAX_BUFF);
	struct sockaddr_in addr;
	socklen_t len=sizeof(addr);
	
	int val = recvfrom(fd,buff,MAX_BUFF,MSG_DONTWAIT,(struct sockaddr *)&addr,&len);
	if(val>0 && !dropMessage(p))
	{
		int size=strlen(buff);
		if(!size)
			return 0;
		int id=(int)buff[size+1];
		if(!strcmp(buff,ACK))
			return HandleACKMsgRecv(id);
		else
			return HandleAppMsgRecv(id,buff,addr,len);
	}
	else{
		return -1;
	}
}

int HandleRetransmit()
{
	if(!unACK_count)
		return 0;
	int i = 0;
	while(i<unACK_count)
	{
		time_t now = time(NULL);
		if ((now - unACKTable[i].time)<=T)
			continue;
		else
		{
			unACKTable[i].buff[strlen(unACKTable[i].buff)+1]=unACKTable[i].id;
			int val = sendto(fd,unACKTable[i].buff,MAX_BUFF,MSG_DONTWAIT,(const struct sockaddr *)&(unACKTable[i].addr),sizeof(unACKTable[i].addr));
			if (val > 0){
				now = time(NULL);
				unACKTable[i].time=now;
				retrans_count++;
			}
			else{
				continue;
			}
		}
		i++;
	}
	return 0;
}

int HandleTransmit()
{
	if(!send_count || unACK_count==MAX_BUFF)
		return 0;
	int i = 0;
	while(i<send_count && unACK_count < MAX_BUFF)
	{
		int val = sendto(fd,sendBuff[i].buff,MAX_BUFF,MSG_DONTWAIT,(const struct sockaddr *)&(sendBuff[i].addr),sizeof(sendBuff[i].addr));
		if (val > 0)
		{
			time_t now = time(NULL);
			
			strcpy(unACKTable[unACK_count].buff,sendBuff[i].buff);
			unACKTable[unACK_count].time=now;
			unACKTable[unACK_count].id=sendBuff[i].id;
			unACKTable[unACK_count].addr=sendBuff[i].addr;

			fla=0;
			i++;
			unACK_count++;
		}
	}
	int j = 0;
	while(j<MAX_BUFF-i){
		sendBuff[j]=sendBuff[j+i];
		j++;
	}
	send_count-=i;
	return 0;
}

int HandleAppMsgRecv(int ID,char *buff,struct sockaddr_in addr,socklen_t len)
{
	
	if(recvIDTable[ID].id==ID)
	{
		ACK[16]=ID;
		if(sendto(fd,ACK,MAX_BUFF,0,(const struct sockaddr *)&addr,len)==-1)
		{
			fprintf(stderr, "ACK resending error\n");
			return -1;
		}
	}
	else if(recvIDTable[ID].id!=-2)
	{
		ACK[16]=ID;

		strcpy(recvBuff[recv_count].buff, buff);
		recvIDTable[ID].id = ID;
		recvIDTable[ID].addr = addr;

		recvBuff[recv_count].addr=addr;
		recv_count++;

		if(sendto(fd,ACK,MAX_BUFF,0,(const struct sockaddr *)&addr,len)==-1)
		{
			fprintf(stderr, "ACK sending error\n");
			return -1;
		}
	}
	else{
		return -1;
	}
}

int HandleACKMsgRecv(int ID)
{
	if(!unACK_count)
		return 0;
	int i, isfound;
	i = isfound =0;
	while(i<unACK_count)
	{
		if(unACKTable[i].id==ID)
			isfound = 1;
		if(isfound)
		{
			if(i+1 < unACK_count)
			{
				unACKTable[i]=unACKTable[i+1];
				strcpy(unACKTable[i].buff,unACKTable[i+1].buff);
			}
			else{
				recvIDTable[ID].id=-2;
				break;
			}
		}
		i++;
	}
	if(isfound)
		unACK_count--;
	return 0;
}


int r_socket(int domain,int type,int protocol)
{
	srand(time(NULL));
	if(type != SOCK_MRP)
	{
		fprintf(stderr, "!! Ensure SOCK_MRP !!\n");
		return -1;
	}
	fd = socket(domain,SOCK_DGRAM,protocol);
	if (fd < 0){
		perror("Failed to create socket\n");
		return -1;
	}
	// initialise the global variables
	fla = 1;
	send_count = recv_count = unACK_count = recv_id_count = retrans_count = 0;
	
	sendBuff=(send_*)malloc(MAX_BUFF*sizeof(send_));
	if(sendBuff==NULL)
	{
		fprintf(stderr, "sendBuff Malloc error\n");
		return -1;
	}
	recvBuff=(recv_*)malloc(MAX_BUFF*sizeof(recv_));
	if(recvBuff==NULL)
	{
		fprintf(stderr, "recvBuff Malloc error\n");
		return -1;
	}
	unACKTable=(unACK_*)malloc(MAX_BUFF*sizeof(unACK_));
	if(unACKTable==NULL)
	{
		fprintf(stderr, "unACKTable Malloc error\n");
		return -1;
	}
	recvIDTable=(recv_id_*)malloc(MAX_BUFF*sizeof(recv_id_));
	if(recvIDTable==NULL)
	{
		fprintf(stderr, "recvIDTable Malloc error\n");
		return -1;
	}
	else
	{
		init_recv_table();
	}

	struct sigaction sa;
	struct itimerval timeout;
	
	memset(&sa,0,sizeof(sa));
	sa.sa_handler=&Signal_Handler;
	
	if(sigaction(SIGALRM,&sa,NULL)==-1)
	{
		fprintf(stderr, "Handler Installation error\n");
		return -1;
	}

	timeout.it_value.tv_sec=T; // waits for T before the starting the signal handler
	timeout.it_value.tv_usec=0;
	timeout.it_interval.tv_sec=T;
	timeout.it_interval.tv_usec=0;

	if(setitimer(ITIMER_REAL,&timeout,NULL)==-1)
	{
		fprintf(stderr, "setitimer enable error\n");
		return -1;
	}
	return fd;
}

int r_bind(int sockfd,const struct sockaddr *addr, socklen_t addrlen)
{
	return bind(sockfd, addr, addrlen);
}

int r_recvfrom(int sockfd,void *buf,size_t len,int flags, struct sockaddr *src_addr,socklen_t addrlen)
{
	for(;;)
	{
		if(!recv_count)
			continue;
		strcpy(buf,recvBuff[0].buff);
		int i=0;
		for(;i+1<recv_count;i++)
			recvBuff[i]=recvBuff[i+1];
		recv_count--;
		return strlen(buf);
	}
}

void sendto_helper(int sockfd,const void *buf,size_t len,int flags, struct sockaddr_in *dest_addr,socklen_t addrlen){
	send_ temp;
	strcpy(temp.buff,buf);
	temp.buff[len]='\0';
	temp.buff[len+1]=send_count;
	temp.id=send_count;
	temp.addr=*dest_addr;
	sendBuff[send_count]=temp;
	send_count++;
}

int r_sendto(int sockfd,const void *buf,size_t len,int flags, struct sockaddr_in *dest_addr,socklen_t addrlen)
{
	while(send_count==MAX_BUFF); // busy wait
	sendto_helper(sockfd, buf, len, flags, dest_addr, addrlen);
	return 0;
}

int r_close(int fd)
{
	if(close(fd)==-1)
	{
		fprintf(stderr, "Socket close error\n");
		return -1;
	}

	struct itimerval timeout;
	timeout.it_value.tv_sec=0;
	timeout.it_value.tv_usec=0;
	if (setitimer(ITIMER_REAL,&timeout,NULL) < 0){
		fprintf(stderr, "setitimer closing error\n");
		return -1;
	}
	free(sendBuff);
	free(recvBuff);
	free(unACKTable);
	free(recvIDTable);
	return 0;
}

