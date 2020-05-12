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
#include <stdbool.h>

//Helper functions
void displayMenu();
void sendSvrMessage();
int recvSvrMessage();
void decodeMessage();
int encodeMessage();
void getInput();
void registerUser();
void printMessages();
int chatMenu();

#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000
#define DELIMITER "-"

int			sock_send,option;
struct sockaddr_in	addr_send,my_addr;
char			text[BUF_SIZE],src_ip[30],buf[BUF_SIZE],filename[70];
int			send_len,bytes_sent,bytes_recv, recv_len;
char userName[50], command[50],name[50],recpName[50];
int select_ret;
fd_set readfds;
char textReply[BUF_SIZE], message[200];
FILE *fptr;
        
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

    //build filename
    strcat(filename,userName);
    strcat(filename,DELIMITER);
    strcat(filename,"notifications.txt");
    
    getInput();
    bool skip=false;
    while(1){
        
        switch(option){
            case 1:
                strcpy(text,"Logout");
                strcpy(command,"QuitApp");
                strcpy(name,userName);
                strcpy(recpName,userName);
                sendSvrMessage();
                printf("Goodbye !\n");
                close(sock_send);
                exit(0);
                break;    
            case 2:
                strcpy(command,"ViewAllContacts");
                strcpy(text,userName);
                strcpy(name,userName);
                strcpy(recpName,userName);
                sendSvrMessage();
                break;
            case 3:
                printf("Are you sure you want to join FunGroup? (yes/no) =>");
                scanf("%s",&textReply);
                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"JoinGroup");
                    strcpy(text,"FunGroup");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
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
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
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
                strcpy(recpName,userName);
                sendSvrMessage();
                break;
            case 6:
                printf("\nEnter broadcast message => ");
                scanf("%s",&textReply);
                //fgets(textReply,BUF_SIZE,std) <-- Not working
                strcpy(command,"WorkGroupBroadcast");
                strcpy(text,textReply);
                strcpy(name,userName);
                strcpy(recpName,userName);
                sendSvrMessage();
                break;
            case 7:
                chatMenu();
                skip=true;
                break;
            case 8:
                printMessages();
                strcpy(command,"NotificationRequest");
                strcpy(text,"NotificationRequest");
                strcpy(name,userName);
                strcpy(recpName,userName);
                sendSvrMessage();
                break;
            case 9:
                printf("Are you sure you want to LEAVE FunGroup? (yes/no) =>");
                scanf("%s",&textReply);
                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"LeaveGroup");
                    strcpy(text,"FunGroup");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }
                break;
            case 10:
                printf("Are you sure you want to LEAVE WorkGroup? (yes/no) =>");
                scanf("%s",&textReply);
                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"LeaveGroup");
                    strcpy(text,"WorkGroup");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"CancelRequest");
                    strcpy(text,"Cancel Request");
                    strcpy(name,userName);
                    strcpy(recpName,userName);
                    sendSvrMessage();
                }
                break;
            default:
                printf("Incorrect option given !\n");
                close(sock_send);
                exit(0);
                break;
        }
        
        if(skip != true){ //remover skip if dont work
            if ( recvSvrMessage()== 0){ //Check if a msg was received
                decodeMessage();

                if((fptr=fopen(filename,"a")) == NULL){ // File stores messages from server
                    printf("File error! \n");
                    close(sock_send);
                    exit(0);
                }

                if (strcmp(command,"Display") == 0){
                    fprintf(fptr,"\nMessage Received: %s\n",text);
                    printf("\nMessage Received: %s\n",text);

                }

                if(strcmp(command,"FunGroupBroadcast") ==0){
                    
                    fprintf(fptr,"\nFunGroup Broadcast: %s\n",text);
                    printf("\nFunGroup Broadcast: %s\n",text);
                }

                if(strcmp(command,"WorkGroupBroadcast") ==0){
                    fprintf(fptr,"\nWorkGroup Broadcast: %s\n",text);
                    printf("\nWorkGroup Broadcast: %s\n",text);
                }

                if (strcmp(command,"NotificationReply")==0){
                    printf("");
                }

                fclose(fptr);
            }else{
                printf("\nNo Message Received:\n");
            
            }

            skip=false;
            
        }
         
        strcpy(command,"");//Ensures old command is not used during next cycle
        strcpy(text,""); //Ensures old message is not shown

        getInput();   
    }

}

void displayMenu(){
    printf("\n1: Exit App \n2: View Contacts\n3: Join FunGroup\n4: Join Workgroup\n");
    printf("5: Send FunGroup Broadcast\n6: Send WorkGroup Broadcast\n7: Chat with someone\n8: Display Notifications\n");
    printf("9: Leave FunGroup\n10: Leave WorkGroup\n");
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

// Message format: command-message-name-receipientname
int encodeMessage(){
    strcpy(buf,command);
    strcat(buf,DELIMITER);
    strcat(buf,text);
    strcat(buf,DELIMITER);
    strcat(buf,name);
    strcat(buf,DELIMITER);
    strcat(buf,recpName);
    

    return (strlen(command) + strlen(text) + strlen(name) + strlen(recpName) +(strlen(DELIMITER) * 3)); //Return length of string to be sent
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
    strcpy(recpName,userName);
    sendSvrMessage();

    if(recvSvrMessage()==0){
        decodeMessage();
        printf("\nMessage Received: %s\n \n",text);

        if(strcmp(text,"Name already exists")==0){ //Names must be unique
            printf("Restart app and use a different name\n");
            close(sock_send);
            exit(0);
        }
    }else{
        printf("\nError, no response from server\n");
        close(sock_send);
        exit(0);
    }
}

void printMessages(){
    if((fptr=fopen(filename,"r"))==NULL){
        printf("Error opening file\n");
        close(sock_send);
        exit(0);
    }

    /*while(fgets(message,sizeof(message),fptr)){//Check for end of file
        printf("\n%s\n",message);
    }*/

    printf("\n****Notifications:****");
    while(fgets(message, sizeof(message), fptr) != NULL) {
        fputs("\n",stdout);
        fputs(message, stdout);
    }

    fclose(fptr);
}

int chatMenu(){
    //Write code to handle chatting
    FILE *fp;
    char file_name[50];
    char recverName[50];
    char msg[200];

    

    printf("Enter the name of the person to chat with => ");
    scanf("%s",&textReply);
    strcpy(command,"ChatRequest");
    strcpy(text,"Request to Chat");
    strcpy(name,userName);
    strcpy(recpName,textReply);
    sendSvrMessage();//Encode and send the message
    printf("Waiting for user to accept....\n");

    bool finished=false;// Turns true if the user finished chatting

    while(!finished){

        if(recvSvrMessage()==0){
            decodeMessage();

            if (strcmp(command,"ChatRequestAccepted")==0){ //ChatWith command can only be used by client if chat request accepted
                //name is the name of the person to chat with
                //write code to chat with the person
                
                strcat(file_name,userName);
                strcat(file_name,"ChatWith");
                strcat(file_name,text);
                strcat(file_name,".txt");

                if((fp=fopen(file_name,"a")) == NULL){ // File for storing chat history
                        printf("File error! \n");
                        close(sock_send);
                        exit(0);
                }

                printf("\nChat request accepted from %s\n",text);
                strcpy(recverName,text);

            
                //Continuously send and recv and store to file
                printf("\nEnter a message or \"Quit\" to stop => ");
                //scanf("%*[^\n]%*c",msg);
                scanf("%s",&msg);
                if(strcmp(msg,"Quit")==0){
                    fclose(fp);
                    finished=true;
                    break;
                }
                strcpy(recpName,recverName);
                strcpy(command,"ChatWith");
                strcpy(name,userName);
                strcpy(text,msg);
                fprintf(fp,"You: %s\n",msg);//Save to file
                sendSvrMessage();

                if(recvSvrMessage()==0){
                    decodeMessage();
                
                    //while((strcmp(msg,"Quit") !=0) && (strcmp(command,"ChatWith")==0)){
                    if (strcmp(command,"ChatWith")==0){// TEST THIS WHEN YOU GET BACK
                        printf("\n%s\n",text);
                        fprintf(fp,"%s: \n",recverName);

                        //Continuously send and recv and store to file
                        printf("\nEnter a message or \"Quit\" to stop => ");
                        //scanf("%*[^\n]%*c",msg);
                        scanf("%s",&msg);
                        if(strcmp(msg,"Quit")==0){
                            fclose(fp);
                            finished=true;// This will stop the while loop
                            break;
                        }
                        strcpy(recpName,recverName);
                        strcpy(command,"ChatWith");
                        strcpy(name,userName);
                        strcpy(text,msg);
                        fprintf(fp,"You: %s\n",msg);//Save to file
                        sendSvrMessage();

                        //recvSvrMessage();
                        //decodeMessage();

                        if(strcmp(text,"Error! User not found")){
                            printf("\n%s\n",text);
                            fclose(fp);
                            break;
                        }
                    
                    }

                    fclose(fp);
                }

            }

            if(strcmp(command,"ChatRequestDeclined")==0){
                printf("\n%s declined your request to chat\n",text);
                return 1;
            }

            if(strcmp(command,"ChatRequest")==0){
                printf("\nDo you want to accept request from %s? (yes/no) => \n",text);
                scanf("%s",&textReply);

                if(strcmp(textReply,"yes")==0){
                    strcpy(command,"ChatRequestAccepted");
                    strcpy(recpName,text);
                    strcpy(text,"Accept Chat Request");
                    strcpy(name,userName);
                    sendSvrMessage();
                }else{
                    strcpy(command,"ChatRequestDeclined");
                    strcpy(recpName,text);
                    strcpy(text,"Decline Chat Request");
                    strcpy(name,userName);
                    sendSvrMessage();
                }
                
            }


        }
        
    
    }

    return 0;
    
}