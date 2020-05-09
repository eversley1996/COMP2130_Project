/* 
    Eversley Francis - 620106294
    Kyle Henry
    Orlando Blagrove
    Ramone Grantson

    Sources:
        -https://overiq.com/c-programming-101/the-sprintf-function-in-c/
        -https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
        -https://flaviocopes.com/c-return-string/
        -Code downloaded from ourvle


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
void getInput();
void registerUser();

#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000
#define DELIMITER "-"

int			sock_send,option;
struct sockaddr_in	addr_send,my_addr;
char			text[1024],src_ip[30],buf[BUF_SIZE];
int			send_len,bytes_sent,bytes_recv, recv_len;
char userName[30], command[30],name[30];
int select_ret;
fd_set readfds;
char textReply[1024];


        
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

    //Register User
    registerUser();
    
    getInput();

    while(1){

        switch(option){
            case 1:
                strcpy(text,"shutdown");
                strcpy(command,"Display");
                strcpy(name,userName);
                sendSvrMessage();
                printf("Goodbye !\n");
                close(sock_send);
                exit(0);
                break;    
            case 2:
                strcpy(command,"ViewAllContacts");
                strcpy(text,userName);
                strcpy(name,userName);
                sendSvrMessage();
                break;
            case 3:
                printf("Are you sure you want to join FunGroup? (yes/no) =>");
                scanf("%s",&textReply);
                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"JoinGroup");
                    strcpy(text,"FunGroup");
                    strcpy(name,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    sendSvrMessage();
                }
                break;
            case 4:
                printf("Are you sure you want to join WorkGroup? (yes/no) =>");
                scanf("%s",&textReply);
                //fgets(textReply,BUF_SIZE,std) <-- Not working
                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"JoinGroup");
                    strcpy(text,"WorkGroup");
                    strcpy(name,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    sendSvrMessage();
                }
                break;
            case 5:
                printf("\nEnter broadcast message => ");
                scanf("%s",&textReply);
                //fgets(textReply,BUF_SIZE,std) <-- Not working
                strcpy(command,"FunGroupBroadcast");
                strcpy(text,textReply);
                strcpy(name,userName);
                sendSvrMessage();
                break;
            case 6:
                break;
            default:
                printf("Incorrect option given !\n");
                close(sock_send);
                exit(0);
                break;
        }
        
        if (recvSvrMessage() == 0){ //Check if a msg was received
            decodeMessage();
            if (strcmp(command,"Display") == 0){
                printf("\nMessage Received: %s\n \n",text);

                if(strcmp(text,"Name already exists")==0){// Check if register name already exists
                    close(sock_send);
                    exit(0);
                }
            }

            if(strcmp(command,"FunGroupBroadcast") ==0){
                
                printf("\nFunGroup Broadcast: %s\n",text);
            }

            if(strcmp(command,"WorkGroupBroadcast") ==0){
                printf("\nWorkGroup Broadcast: %s\n",text);
            }
        }else{
            printf("No Message Received:");
        }

        strcpy(command,"");//Ensures old command is not used during next cycle
        strcpy(text,""); //Ensures old message is not shown

        
        getInput();   
    }

}

void displayMenu(){
    printf("\n1: Quit \n2: View Contacts\n3: Join FunGroup\n4: Join Workgroup\n");
    printf("5: Send FunGroup Broadcast\n6: Send WorkGroup Broadcast\n7: Display Messages\n8: Chat with someone\n");
}

void getInput(){
    printf("\nPlease select an option:\n");
    displayMenu();
    printf("=> ");
    scanf("%d",&option);
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

// Message format: command-message-name
int encodeMessage(){
    strcpy(buf,command);
    strcat(buf,DELIMITER);
    strcat(buf,text);
    strcat(buf,DELIMITER);
    strcat(buf,name);
    

    return (strlen(command) + strlen(text) + strlen(name)+(strlen(DELIMITER) * 3)); //Return length of string to be sent
}

//Message format: command-message
void decodeMessage(){
    strcpy(command,strtok(buf, DELIMITER));      /* Get the command (first message)*/
    strcpy(text,strtok(NULL, DELIMITER));    /* Get the message*/
}

void registerUser(){
    printf("\n**** Welcome to IM Pro ****\n");
    printf("\nPlease enter a name to use: => ");
    scanf("%s",&userName);

    if(strlen(userName) < 2){
        printf("\nCannot use this appplication without a valid name. Goodbye \n");
        close(sock_send);
        exit(0);
    }

    strcpy(command,"Register");
    strcpy(text,userName);
    strcpy(name,userName);
    sendSvrMessage();

    if(recvSvrMessage()==0){
        decodeMessage();
        printf("\nMessage Received: %s\n \n",text);
    }else{
        printf("\nError, no response from server\n");
        close(sock_send);
        exit(0);
    }
}