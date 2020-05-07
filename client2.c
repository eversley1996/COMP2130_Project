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
int recvSvrMessage();
void decodeMessage();
int encodeMessage();

#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000
#define DELIMITER "-"

int			sock_send,option;
struct sockaddr_in	addr_send,my_addr;
char			text[1024],src_ip[30],buf[BUF_SIZE];
int			send_len,bytes_sent,bytes_recv, recv_len;
char userName[30], command[30];
        
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

    printf("Welcome to IM Pro ! \n");
    printf("Please enter a name to use: => ");
    scanf("%s",&userName);

    if(strlen(userName) < 2){
        printf("Cannot use this appplication without a valid name. Goodbye \n");
        close(sock_send);
        exit(0);
    }

    printf("Please select an option:\n");
    displayMenu();
    printf("=> ");
    scanf("%d",&option);

    while(1){
        
        switch(option){
            case 0:
                strcpy(text,"shutdown");
                strcpy(command,"Display");
                sendSvrMessage();
                printf("Goodbye !\n");
                close(sock_send);
                exit(0);
                break;
            case 1:
                strcpy(command,"Register");
                strcpy(text,userName);
                sendSvrMessage();
                break;
            case 2:
                strcpy(command,"ViewAllContacts");
                strcpy(text,userName);
                sendSvrMessage();
                break;

            
            default:
                printf("Incorrect option given !\n");
                break;
        }

        if(recvSvrMessage() == 0){ //Check if a msg was received
            decodeMessage();
        }

        if (strcmp(command,"Display") == 0){
            printf("\nMessage Received: %s\n \n",text);
        }

        printf("Please select an option:\n");
        displayMenu();
        printf("=> ");
        scanf("%d",&option);
    }


}

void displayMenu(){
    printf("0: Quit \n1: Register\n2: View Contacts\n");
}

void sendSvrMessage(){
    send_len=encodeMessage();
    bytes_sent=sendto(sock_send, buf, send_len, 0,(struct sockaddr *) &addr_send, sizeof(addr_send));

}

int recvSvrMessage(){ //Return 1 if no message was received
    recv_len=sizeof(my_addr);
    bytes_recv=recvfrom(sock_send,buf,BUF_SIZE,0,(struct sockaddr *)&my_addr,&recv_len);
    if (bytes_recv>0){	/* what was sent? */
        buf[bytes_recv]='\0';
        return 0;
        //printf("From %s: received: %s\n",inet_ntoa(addr_send.sin_addr),buf);
    }else{
        return 1;
    }
}

// Message format: command-message
int encodeMessage(){
    strcpy(buf,command);
    strcat(buf,DELIMITER);
    strcat(buf,text);
    

    return (strlen(command) + strlen(text) + (strlen(DELIMITER) * 3)); //Return length of string to be sent
}

void decodeMessage(){
    strcpy(command,strtok(buf, DELIMITER));      /* Get the command (first message)*/
    strcpy(text,strtok(NULL, DELIMITER));    /* Get the message*/
}