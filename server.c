#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE	1024
#define LISTEN_PORT	60000

const char* recvMessage();
void sendMessage();

int sent_msg,send_len;
int			sock_recv;
struct sockaddr_in	my_addr;
int			i;
fd_set	readfds,active_fd_set,read_fd_set;
struct timeval		timeout={0,0};
int			incoming_len;
struct sockaddr_in	remote_addr;
int			recv_msg_size;
char			buf[BUF_SIZE];
int			select_ret;

int main(int argc, char *argv[]){
    

    //printf("Hi\n");
            /* create socket for receiving */
    sock_recv=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_recv < 0){
        printf("socket() failed\n");
        exit(0);
    }
        /* make local address structure */
    memset(&my_addr, 0, sizeof (my_addr));	/* zero out structure */
    my_addr.sin_family = AF_INET;	/* address family */
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* current machine IP */
    my_addr.sin_port = htons((unsigned short)LISTEN_PORT);
        /* bind socket to the local address */
    i=bind(sock_recv, (struct sockaddr *) &my_addr, sizeof (my_addr));
    if (i < 0){
        printf("binding() failed\n");
        exit(0);
    }
    
    FD_ZERO(&readfds);		/* zero out socket set */
    FD_SET(sock_recv,&readfds);	/* add socket to listen to */
        /* listen ... */
    while (1){
        FD_ZERO(&readfds);		/* zero out socket set */
        FD_SET(sock_recv,&readfds);	/* add socket to listen to */
        
        printf("Server listening...\n");
        read_fd_set = active_fd_set;
        select_ret=select(sock_recv+1,&readfds,NULL,NULL,NULL);
        /*select_ret=select(sock_recv+1,&readfds,NULL,NULL,&timeout);*/

        if (select_ret > 0){/* anything arrive on any socket? */

            recvMessage();    
        }
        
        if (strcmp(buf,"shutdown") == 0)
            break;

        if (strcmp(buf,"Register")==0){
            strcpy(buf,"Enter yours Name:");
            sendMessage();
            printf("%s\n",recvMessage());
        }
    }

    close(sock_recv);
}

const char* recvMessage(){
    incoming_len=sizeof(remote_addr);	/* who sent to us? */
    recv_msg_size=recvfrom(sock_recv,buf,BUF_SIZE,0,(struct sockaddr *)&remote_addr,&incoming_len);
    if (recv_msg_size > 0){	/* what was sent? */
        buf[recv_msg_size]='\0';
        printf("From %s: received: %s\n",inet_ntoa(remote_addr.sin_addr),buf);
        //strcpy(msg,buf);
        return buf;
    }else{
        strcpy(buf,"");
        return buf;
    }
}

void sendMessage(){
    send_len= strlen(buf);
    sent_msg=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
}