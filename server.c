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
char		buf[BUF_SIZE], text[1024],test[1024], command[30], name[50];
int			select_ret, user_index;
bool userFound;
FILE *fptr;

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

    FD_ZERO(&readfds);		/* zero out socket set */
    FD_SET(sock_recv,&readfds);	/* add socket to listen to */
    

    /* listen ... */
    while (1){
        FD_ZERO(&readfds);		/* zero out socket set */
        FD_SET(sock_recv,&readfds);	/* add socket to listen to */

        printf("\nServer listening...\n");
        active_fd_set=readfds;
        
        select_ret=select(sock_recv+1,&active_fd_set,NULL,NULL,NULL);
        
        if(FD_ISSET(sock_recv,&active_fd_set)){
    
            if (select_ret > 0){/* anything arrive on any socket? */
                recvMessage();
                decodeMessage(); //Get the message and the command

                if (strcmp(text,"shutdown") == 0)
                    break;

                if (strcmp(command,"Register")==0){ 
                    //write code to register name stored in "text"
                    strcpy(command,"Display");
                    strcpy(test,"");

                    for(int j=0; j<10; j++){// Check if name already exists
                            if(strcmp(userList[j].name,text) ==0){
                                strcpy(test,"Name already exists\n");
                                break;
                            }    
                    }
                    int position= newUserPosition();// Find position in the array for new user
                    if((position >= 0) && (strcmp(test,"")==0)){// Add user info to the list
                        strcpy(userList[position].ip,inet_ntoa(remote_addr.sin_addr));
                        strcpy(userList[position].name,text);
                        userList[position].WorkGroup = false;
                        userList[position].FunGroup = false;
                        userList[position].socket = remote_addr;

                        strcpy(text,"User Registered");//Message to be send back to client

                        
                    }else{
                        if(strcmp(test,"")==0){
                            strcpy(text,"No Space,cannot register user");
                        }else{
                            strcpy(text,test);// Name exists
                        }
                        
                    }
                    sendMessage();//Send message to client
                    
                }

                if (strcmp(command,"ViewAllContacts")==0){
                
                    strcpy(text,"\n\nList of Users: \n");//Ensure string is blank
                    viewAllUsers(text);
                    strcpy(command,"Display");
                    sendMessage();

                }

                if (strcmp(command,"CancelRequest")==0){
                    strcpy(command,"Display");
                    strcpy(text,"Request Cancelled\n");
                    sendMessage();
                }

                if (strcmp(command,"JoinGroup")==0){

                    strcpy(command,"Display");// Change command for sending
                    if(strcmp(text,"WorkGroup")==0){
                        userFound=false;
                        for (int x=0; x<10; x++){
                            if(strcmp(userList[x].name,name)==0){ // "name" stores name of client
                                userList[x].WorkGroup=true;
                                userFound=true;
                                strcpy(text,"Group request successful\n");
                                sendMessage(); //Sends string stored in text variable
                                break; //Dont bother checking the others
                            }
                        }
                        if(userFound==false){
                            strcpy(text,"Request unsucessful\n");
                            sendMessage();
                        }
                    }

                    if(strcmp(text,"FunGroup")==0){
                        userFound=false;
                        for (int x=0; x<10; x++){
                            if(strcmp(userList[x].name,name)==0){ // "name" stores name of client
                                userList[x].FunGroup=true;
                                userFound=true;
                                strcpy(text,"Group request successful\n");
                                sendMessage(); //Sends string stored in text variable
                                break; //Dont bother checking the others
                            }
                        }
                        if(userFound==false){
                            strcpy(text,"Request unsucessful\n");
                            sendMessage();
                        }
                    }
                    
                
                }

                
                if (strcmp(command,"FunGroupBroadcast")==0){
                    
                    userFound=false; 
                    for(int x=0; x<10; x++){ //check if user is a group member
                        if((strcmp(userList[x].name,name)==0) && userList[x].FunGroup==true){
                            userFound=true;
                            break;//Stop searching once found
                        }
                    }

                    if(userFound){
                        strcpy(test,text);// test temporarily holds the message to be sent
                        strcpy(text,"FunGroup Broadcast sent\n");
                        strcpy(command,"Display");
                        sendMessage(); //Send confirmation back to client

                        /* This code sends the message to each member of group*/
                        strcpy(text,""); //Ensure text is blank
                        strcat(text,name);
                        strcat(text,": ");
                        strcat(text,test);
                        strcpy(command,"FunGroupBroadcast");

                        for(int j=0; j<10; j++){//Check if they are apart of fungroup and send to the socket
                            if(userList[j].FunGroup == true){
                                send_len = encodeMessage();
                                //Send to each user socket 
                                sent_msg=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &userList[j].socket, sizeof(userList[j].socket));
                            }
                        }

                    }else{
                        strcpy(text,"You must be a member before you broadcast.\n");
                        sendMessage();
                    }
                    

                }

                if (strcmp(command,"WorkGroupBroadcast")==0){
                    
                    userFound=false; 
                    for(int x=0; x<10; x++){ //check if user is a group member
                        if((strcmp(userList[x].name,name)==0) && userList[x].WorkGroup==true){
                            userFound=true;
                            break;//Stop searching once found
                        }
                    }

                    if(userFound){
                        strcpy(test,text);// test temporarily holds the message to be sent
                        strcpy(text,"WorkGroup Broadcast sent\n");
                        strcpy(command,"Display");
                        sendMessage(); //Send confirmation

                        /* This code sends the message to each member of group*/
                        strcpy(text,""); //Ensure text is blank
                        strcat(text,name);
                        strcat(text,": ");
                        strcat(text,test);
                        strcpy(command,"WorkGroupBroadcast");
                        for(int j=0; j<10; j++){
                            if(userList[j].FunGroup == true){
                                send_len = encodeMessage();
                                //Send to each user socket 
                                sent_msg=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &userList[j].socket, sizeof(userList[j].socket));
                            }
                        }

                    }else{
                        strcpy(text,"You must be a member before you broadcast.\n");
                        sendMessage();
                    }
                    

                }



                //CHECK FOR OThER COMMANDS HERE

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
    strcpy(name,strtok(NULL, DELIMITER)); /*Get the name of recipient*/
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
    int number=1;
    strcat(text,"\nAvailable Users:");
    for (i=0;i < 10; i++){
        if (strcmp(userList[i].ip, "") != 0){
            sprintf(&text[strlen(text)], "\n%d: %s",number,userList[i].name);
            number++;
        }
    }
    printf("%s",text);
} 