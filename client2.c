/* 
    Eversley Francis - 620106294
    Kyle Henry
    Orlando Blagrove
    Ramone Grantson

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

void displayMenu();
void sendSvrMessage();
void recvSvrMessage();

#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000

int			sock_send,option;
struct sockaddr_in	addr_send,my_addr;
char			text[80],src_ip[30],buf[BUF_SIZE];
int			send_len,bytes_sent,bytes_recv, recv_len;
        
int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Incorecct arguments given, please enter an IP to use\n");
        exit(0);
    }
    strcpy(src_ip,argv[1]); //Get the IP from the command line
    
    /* create socket for sending data */
    sock_send=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_send < 0){
        printf("socket() failed\n");
        exit(0);
    }

    struct sockaddr_in localaddr;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(src_ip);
    localaddr.sin_port = SERVER_PORT;
    bind(sock_send, (struct sockaddr *)&localaddr, sizeof(localaddr));// Associate the socket with an IP

    /* fill the address structure for sending data */
    memset(&addr_send, 0, sizeof(addr_send));  /* zero out structure */
    addr_send.sin_family = AF_INET;  /* address family */
    addr_send.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr_send.sin_port = htons((unsigned short)SERVER_PORT);

    
    while(1){
        printf("Welcome to IM Pro ! \n");
        printf("Please select an option:\n");
        displayMenu();
        scanf("%d",&option);
        switch(option){
            case 0:
                strcpy(text,"shutdown");
                strcpy(buf,text);
                sendSvrMessage();
                printf("Goodbye !\n");
                close(sock_send);
                exit(0);
                break;
            case 1:
                strcpy(text,"Register");
                strcpy(buf,text);
                sendSvrMessage();

                //Wait on response from server
                recvSvrMessage();
                scanf("%s",&text);
                strcpy(buf,text);
                sendSvrMessage();
                recvSvrMessage();
                break;
            
            default:
                printf("Incorrect option given !\n");
                break;
        }

    
        //recv_len=sizeof(my_addr);
        //bytes_recv=recvfrom(sock_send,buf,BUF_SIZE,0,(struct sockaddr *)&my_addr,&recv_len);
        //if (bytes_recv>0){	/* what was sent? */
        //        buf[bytes_recv]='\0';
        //        printf("From %s: received: %s\n",inet_ntoa(addr_send.sin_addr),buf);
        //}
        
    }


    
}

void displayMenu(){
    printf("0: Quit \n1: Register\n2: View Contacts\n");
}

void sendSvrMessage(){
    send_len=strlen(text);
    bytes_sent=sendto(sock_send, buf, send_len, 0,(struct sockaddr *) &addr_send, sizeof(addr_send));

}

void recvSvrMessage(){
    recv_len=sizeof(my_addr);
    bytes_recv=recvfrom(sock_send,buf,BUF_SIZE,0,(struct sockaddr *)&my_addr,&recv_len);
    if (bytes_recv>0){	/* what was sent? */
        buf[bytes_recv]='\0';
        printf("From %s: received: %s\n",inet_ntoa(addr_send.sin_addr),buf);
    }
}