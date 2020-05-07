#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdbool.h>

#define BUF_SIZE	1024
#define LISTEN_PORT	60000
#define DELIMITER "-"

const char* recvMessage();
void sendMessage();
int encodeMessage();
void decodeMessage();
int newUserPosition();
void viewAllUsers(char *text);

typedef struct {
    
   struct   sockaddr_in socket;
   char     ip[20];
   char     name[30];
   bool      WorkGroup;
   bool      FunGroup;
} UserInfo;

UserInfo userList[10] = {0};

int sent_msg,send_len;
int			sock_recv;
struct sockaddr_in	my_addr;
int			i;
fd_set	readfds,active_fd_set,read_fd_set;
struct timeval		timeout={0,0};
int			incoming_len;
struct sockaddr_in	remote_addr;
int			recv_msg_size;
char		buf[BUF_SIZE], text[1024], command[30];
int			select_ret, user_index;
bool userFound;

int main(int argc, char *argv[]){
    
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
    

    /* listen ... */
    while (1){
        FD_ZERO(&readfds);		/* zero out socket set */
        FD_SET(sock_recv,&readfds);	/* add socket to listen to */
        
        if(FD_ISSET(sock_recv,&readfds)){
            printf("Server listening...\n");
            read_fd_set = active_fd_set;
            select_ret=select(sock_recv+1,&readfds,NULL,NULL,NULL);
            /*select_ret=select(sock_recv+1,&readfds,NULL,NULL,&timeout);*/

            if (select_ret > 0){/* anything arrive on any socket? */

                recvMessage();    
            }

            decodeMessage(); //Get the message and the command

            if (strcmp(text,"shutdown") == 0)
                break;

            if (strcmp(command,"Register")==0){
                //write code to register name stored in "text"
                strcpy(command,"Display");

                if(newUserPosition() >= 0){
                    strcpy(userList[i].ip,inet_ntoa(remote_addr.sin_addr));
                    strcpy(userList[i].name,text);
                    userList[i].WorkGroup = false;
                    userList[i].FunGroup = false;
                    userList[i].socket = remote_addr;

                    printf("New User Added: %s at %s\n",userList[i].name,userList[i].ip);
                    strcpy(text,"User Registered");//Message to be send back to client

                    for(int j=0; j<10; j++){
                        if(strcmp(userList[j].name,"") !=0)
                            printf("%s\n",userList[j].name);
                    }
                }else{
                    strcpy(text,"No Space,cannot register user");

                }
                sendMessage();
                //printf("%s\n",recvMessage());
            }

            if (strcmp(command,"ViewAllContacts")==0){

                //Check if user is registered
                for (int x=0; x<10;x++){
                    if(strcmp(userList[x].name,text) == 0){
                        userFound=true;
                        break; //Stop looking if user found
                    }
                }
                
                if(userFound){
                    strcpy(text,"\n\nList of Users: \n");//Ensure string is blank
                    viewAllUsers(text);
                }else{
                    strcpy(text,"You must register before viewing contacts ! \n");
                }

                strcpy(command,"Display");
                sendMessage();

            }
        }
        
    }

    close(sock_recv);

    return 0;
}

const char* recvMessage(){
    incoming_len=sizeof(remote_addr);	/* who sent to us? */
    recv_msg_size=recvfrom(sock_recv,buf,BUF_SIZE,0,(struct sockaddr *)&remote_addr,&incoming_len);
    if (recv_msg_size > 0){	/* what was sent? */
        buf[recv_msg_size]='\0';
        printf("From %s received: %s\n",inet_ntoa(remote_addr.sin_addr),buf);
        //strcpy(msg,buf);
        return buf;
    }else{
        strcpy(buf,"");
        return buf;
    }
}

void sendMessage(){
    send_len= encodeMessage();
    sent_msg=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
}

//Format: command-message
void decodeMessage(){
    strcpy(command,strtok(buf, DELIMITER));      /* Get the command (first message)*/
    strcpy(text,strtok(NULL, DELIMITER));    /* Get the message*/
}

//Format: command-message
int encodeMessage(){
    strcpy(buf,command);
    strcat(buf,DELIMITER);
    strcat(buf,text);

    return (strlen(command) + strlen(text) + (strlen(DELIMITER) * 3)); //Return length of string to be sent
}

int newUserPosition(){
    for (int i=0; i<10; i++){
        if(strcmp(userList[i].name,"") == 0){
            return i;
        }
    }
    return -1;
}

void viewAllUsers(char *text){

    memset(text,0,sizeof(text)); //Ensure string is blank
    
    for (i=0;i < 10; i++){
        if (strcmp(userList[i].name, "") != 0){
            sprintf(&text[strlen(text)], "%d: %s\n",i+1,userList[i].name);
        }
    }
    printf("Available Users: %s\n",text);
}