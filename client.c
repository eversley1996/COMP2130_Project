/*          COMP2130 - System Programming Project 2

                 The Bootleg C WhatsApp Clone
   
                Submission Date: April 7, 2017
   
                        Group Members:

                -> Odane Barnes  - 620087815
                -> Andrew Hylton - 620088048
                -> Shanell Brown - 620085814     

Note: Please run the server application with the 'tee' bash command to enable piping of the server's output 
      to stdout and to an output file simultaneously. This is done to enable logging of the server.
      
      You may achieve this by calling the program as such on the command line:
      
      stdbuf -o 0 ./server 2>&1 | tee serverlog.txt

Acknowledgements:   Code in this project was based on: 
                        -> Sir's provided multi-client and server code
                        -> This C tutorial on FD_SET() and select() : http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
                        -> and many many StackOverflow answers.

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>

#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000
#define DELIMITER   "%^&*"

// Global Variables accessible from all functions 
struct      sockaddr_in	addr_send;
struct      timeval tv = {10, 0};   // Timeout for 10 seconds
int			i, select_ret, incoming_len, sock_send, recv_msg_size, send_len, bytes_sent;
int         status = 0, WorkGroup = 0, FunGroup = 0, runAgain; // Flags to indicate a condition
char        user_ip[30], src_ip[30], dest_ip[30], command[30], username[30];
char        buf[BUF_SIZE], message[1024], Userinput, chat_username[30], chat_ip[30];
fd_set	    readfds;
FILE        *fp;
char        WorkGroupChatFileName[60], FunGroupChatFileName[60]; 

typedef struct {
   char     ip[20];
   char     name[30];
   char     file_name[50];
} ContactDetails;

ContactDetails users[20] = {0}; // Stores list of active users' IP addresses and names

//Declaration of helper Functions and subroutines
char getInputFromStdin();
void ServerRegistration();
void ViewAllClients();
int EncodeMessage();
void DecodeMessage();
void MainMenu(char *ip_address);
void SendMessageToServer();
int ReceiveMessageFromServer();
int JoinGroup();
int LeaveGroup();
void ConnectToChat();
void ClientChat(char *name_of_file, char *Type);
void SendGroupBroadcastMessage(char *Group);
int ClientLookUp(char *dest_ip, char *username);
int FindEmptySpace();
int ViewAllContacts();
int ReadMessages(char *name_of_file);
void WriteMessageToFile(char *name_of_file, char *sender, char *sender_message);
int SearchContacts(char *username);


int main(int argc, char *argv[]) {
    
    // Terminates program if no IP address to bind to is supplied
    if (argc < 2){
        printf("Please enter a localhost IP address to run this client from\n");
        printf("You can specify an address such as 127.0.0.2, 127.0.0.3, 127.0.0.4, etc.\n");
        exit(1);
    }

    /* create socket for sending data */
    sock_send=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_send < 0){
        printf("socket() failed\n");
        exit(0);
    }

    // Copies the specified IP address to these variables for later usage
    strcpy(src_ip,argv[1]);
    strcpy(user_ip,argv[1]);

    // Bind to a specific network interface (and optionally a specific local port)
    struct sockaddr_in localaddr;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(src_ip);
    localaddr.sin_port = 60000;  // Any local port will do
    bind(sock_send, (struct sockaddr *)&localaddr, sizeof(localaddr));

    /* fill the address structure for sending data */
    memset(&addr_send, 0, sizeof(addr_send));  /* zero out structure */
    addr_send.sin_family = AF_INET;  /* address family */
    addr_send.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr_send.sin_port = htons((unsigned short)SERVER_PORT);

    FD_ZERO(&readfds);		    /* zero out socket set */
    FD_SET(sock_send,&readfds);	/* add socket to listen to */

    ServerRegistration();   /* Completes Registration Process with server */

    /* This is done to allow unique filenames for each user's instance 
       of the client program running. This is done to avoid data races, 
       data duplications and write collisions of broadcast messages 
       when multiple instances of the client program is run from the same location. */
    strcpy(WorkGroupChatFileName,username);
    strcat(WorkGroupChatFileName,"WorkGroupMessages");
    strcpy(FunGroupChatFileName,username);
    strcat(FunGroupChatFileName,"FunGroupMessages");

    MainMenu(user_ip);    /* Prints Main Menu    */    
    while(1){
        FD_SET(sock_send,&readfds);	/* Server Messages  */
        FD_SET(0, &readfds);         /* Input from Stdin */
    
        select_ret=select(sock_send+1, &readfds, NULL, NULL, NULL);

        /* This branch handles messages from server and processes: 
            - Group broadcast messages 
            - When a user wants to you as a contact
            - When a contact sends you a message                */
        if(FD_ISSET(sock_send,&readfds)){
            if ( (status = ReceiveMessageFromServer() ) == 1){  
                
                // List of possible branches for the messages received from the server
                if (strcmp(command,"ConnectToChat") == 0){
                        printf("User %s wants to chat with you. Add as a Contact (Y/N)? ",message);
                        Userinput = getInputFromStdin();
                        
                        if (Userinput == 'Y'){
                            strcpy(dest_ip,src_ip);        // Sets IP address to send response to 
                            strcpy(command,"ChatAccepted");
                            
                            SendMessageToServer();
                            
                            i = FindEmptySpace();
                            if (i != -1) {
                                // Adding new user to contacts to enable chat
                                strcpy(users[i].ip,src_ip);
                                strcpy(users[i].name,message);
                                strcpy(buf,username);
                                strcat(buf,"ChatWith");
                                strcat(buf,users[i].name);
                                strcpy(users[i].file_name,buf); // This file will store all chat convos between users
                                
                                printf("Successfully Added New Contact %s at %s\n",users[i].name,users[i].ip);
                            }
                            else {
                                printf("Error occurred - User was not added\n");
                            }
                            
                            printf("\n\nPress Enter to return to main menu...");
                            getchar();
                            MainMenu(user_ip);
                        }                            
                        else{
                            strcpy(command,"ChatDeclined");
                            SendMessageToServer();
                        }
                } // End of ConnectToChat Branch

                // Writes Work group broadcast messages received to file if you are subscribed
                if ((strcmp(command,"WorkGroupBroadcast") == 0) && (WorkGroup == 1)){
                    WriteMessageToFile(WorkGroupChatFileName, dest_ip, message);
                } // End of WorkGroupBroadcast Branch
                
                // Writes Fun group broadcast messages received to file if you are subscribed
                if ((strcmp(command,"FunGroupBroadcast") == 0) && (FunGroup == 1)){
                    WriteMessageToFile(FunGroupChatFileName, dest_ip, message);
                } // End of FunGroupBroadcast Branch
                
                // Looks up contact details and writes messages received to file for the identified contact
                if ((strcmp(command,"ClientChatMessage") == 0)){
                    if (strcmp(message,"QuitChat") != 0){
                        i = ClientLookUp(src_ip, "");
                        if (i != -1){
                            fp=fopen(users[i].file_name, "a");
                            fprintf(fp,"%s: %s\n",users[i].name,message);
                            fclose(fp);
                        }
                        else{
                            printf("Error - User not Found to save chat\n");
                        } 
                    }                     
                }
            }// End of if ( (status = ReceiveMessageFromServer() ) == 1)
            else{
                printf("Error Reading Message from server\n");
            }
        } // End of if (FD_ISSET(sock_send,&readfds))
   
        /* This branch handles input from the user from stdin and processes: 
            - The User Interface 
            - When a user wants to you as a contact
            - When a contact sends you a message                */
        if(FD_ISSET(0, &readfds)){
        Userinput = getInputFromStdin();
                
        system("clear");
        switch(Userinput) {
            // View all clients branch
            case 'V' :  
                ViewAllClients();
                printf("Press Enter to return to main menu...");
                getchar();
                break;
            
            // Add a client to contacts branch
            case 'A' :  
                ViewAllClients();
                ConnectToChat();
                getchar();
                break;
            
            // Send message to a contact branch
            case 'S' :  
                if (ViewAllContacts() != -1){
                    printf("\n\nPlease enter the name of the contact to send a message: ");
                    fgets (message, 30, stdin);
                    message[strlen(message)-1] = '\0';
                    
                    if (SearchContacts(message) != -1 ){
                        i = ClientLookUp(chat_ip, "");
                        system("clear");
                        printf("\t\t\tChat with %s\n\n\t\tEnter 'QuitChat' to return to main menu\n\n",users[i].name);
                        ClientChat(users[i].file_name, "ClientChatMessage");
                    }
                    else{
                        printf("Username not found\n\n");
                    }
                }
                else{
                    printf("No contacts found\n\n");
                }

                printf("\n\nPress Enter to return to main menu...");
                getchar();
                break;
            
            // Groups Menu
            case 'G' :
                runAgain = 1;
                while(runAgain){
                    system("clear");

                    printf("Groups Menu!\n" );
                    printf("[J]oin a group\n");
                    printf("[L]eave a Group\n");
                    printf("[V]iew Group Messages\n");
                    printf("[S]end Broadcast to a group\n\n");
                    printf("[M]ain Menu\n");

                    Userinput = getInputFromStdin();
                    system("clear");
                    switch (Userinput) {
                        // Join a group branch
                        case 'J' :
                            printf("Please select a group to join\n\n");
                            printf("Join [W]orking Group\n");
                            printf("Join [F]un Group\n");
                            Userinput = getInputFromStdin();
                            
                            system("clear");
                            switch (Userinput) {
                                // Work Group branch
                                case 'W' :
                                    strcpy(command,"JoinGroup");
                                    strcpy(message,"WorkGroup"); 
                                    if (JoinGroup() == 1)
                                        WorkGroup = 1;
                                                                        
                                    printf("\nPress Enter to return to Groups menu..." );
                                    getchar();
                                    break;
                                
                                // Fun Group branch
                                case 'F' :
                                    strcpy(command,"JoinGroup");
                                    strcpy(message,"FunGroup"); 
                                    if (JoinGroup() == 1)
                                        FunGroup = 1;
                                                                                                        
                                    printf("\nPress Enter to return to Groups menu..." );
                                    getchar();
                                    break;
                                
                                default :
                                    printf("Invalid Input\nPress Enter to return to Groups menu..." );
                                    getchar();
                            }
                        break;

                        case 'L' :
                            printf("Please select a group to leave\n\n");
                            printf("Leave [W]orking Group\n");
                            printf("Leave [F]un Group\n");
                            Userinput = getInputFromStdin();
                            
                            system("clear");
                            switch (Userinput) {
                                // Work Group branch
                                case 'W' :
                                    strcpy(command,"LeaveGroup");
                                    strcpy(message,"WorkGroup"); 
                                    if (LeaveGroup() == 1)
                                        WorkGroup = 0;
                                                             
                                    printf("\nPress Enter to return to Groups menu..." );
                                    getchar();
                                    break;
                                
                                // Fun Group branch
                                case 'F' :
                                    strcpy(command,"LeaveGroup");
                                    strcpy(message,"FunGroup"); 
                                    if (LeaveGroup() == 1)
                                        FunGroup = 0;

                                    printf("\nPress Enter to return to Groups menu..." );
                                    getchar();
                                    break;
                                
                                default :
                                    printf("Invalid Input\nPress Enter to return to main menu..." );
                                    getchar();
                            }
                        break;

                        case 'S' :
                            printf("Please select a group to send a message\n\n");
                            printf("Send message to [W]orking Group\n");
                            printf("Send message to [F]un Group\n");
                            Userinput = getInputFromStdin();
                            
                            system("clear");
                            switch (Userinput) {
                                // Work Group branch
                                case 'W' :
                                    SendGroupBroadcastMessage("Work");
                                    getchar();
                                    break;
                                
                                // Fun Group branch
                                case 'F' :
                                    SendGroupBroadcastMessage("Fun");
                                    getchar();
                                    break;
                                
                                default :
                                    printf("Invalid Input\nPress Enter to return to Groups menu..." );
                                    getchar();
                            }
                        break;

                        case 'V' :
                            printf("Please select a group to view messages\n\n");
                            printf("Send message to [W]orking Group\n");
                            printf("Send message to [F]un Group\n");
                            Userinput = getInputFromStdin();
                            
                            system("clear");
                            switch (Userinput) {
                                // Work Group branch
                                case 'W' :
                                    if (WorkGroup == 1){
                                        if (ReadMessages("WorkGroupMessages") != -1){
                                            printf("Insert logic to send here");
                                        }
                                        else{
                                            printf("Error - File not found");
                                        }
                                    }
                                    else{
                                        printf("Error you are not subscribed to the group");
                                    }                                    

                                    printf("\n\nPress Enter to return to main menu");
                                    getchar();
                                    break;
                                
                                // Fun Group branch
                                case 'F' :
                                    if (FunGroup == 1){
                                        if (ReadMessages("FunGroupMessages") != -1){
                                            printf("Insert logic to send here");
                                        }
                                        else{
                                            printf("Error - File not found");
                                        }
                                    }
                                    else{
                                        printf("Error you are not subscribed to the group");
                                    }                                    

                                    printf("\n\nPress Enter to return to main menu");                                    
                                    getchar();
                                    break;
                                
                                default :
                                    printf("Invalid Input\nPress Enter to return to Groups menu..." );
                                    getchar();
                            }
                        break;

                        // Return to main menu branch
                        case 'M' :
                            runAgain = 0;
                            break;

                        default :
                            printf("Invalid Input\nPress Enter to return to Groups menu..." );
                            getchar();
                        }
                    }                
                    break;
            
            // Quit Program branch
            case 'Q' :
                close(sock_send);
                system("clear");                
                exit(0);
            
            default :
                printf("Invalid Input\nPress Enter to return to main menu..." );
                getchar();
        
        } // End of main Switch statement     
        
        MainMenu(user_ip);   

        } // End of if(FD_ISSET(0, &readfds))         
    } // end of while loop
} // end of main

/* Obtains a single character from the user and clears the extra characters remaining in stdin such 
   the newline character '\n' or any other characters remaining. This clearing of stdin is done to 
   avoid the subsequent case statements to read the remaining characters as input for processing.   */
char getInputFromStdin(){
    char clear_stdin;
    char character = fgetc(stdin);
    character = toupper(character);

    // Clears any other extra characters from stdin
    while ((clear_stdin = getchar()) != '\n' && clear_stdin != EOF) { } 
    return character;
}

/* Handles the server registration process which has to be done before the user can use the service */
void ServerRegistration(){
    printf("Enter username to register on network as:\n" );
    fgets (username, 30, stdin);
    username[strlen(username)-1] = '\0';
    strcpy(dest_ip,SERVER_IP);
    strcpy(command,"NewUser");
    strcpy(message,username);                

    SendMessageToServer();

    // Listens for response from server for a message
    select_ret=select(sock_send+1,&readfds,NULL,NULL,NULL);    
    if ( (status = ReceiveMessageFromServer() ) == 1){
        if (strcmp(command,"NewUserSuccess") == 0){
        printf("Successfully Registered on network\nPress Enter to continue...");
        getchar();
        system("clear");
        }
    }
    else{
        printf("Registration Failed\nProgram Exiting\n");
        exit(1);
    }
}

/* Sends command to retrieve list of available users to server */
void ViewAllClients(){
    strcpy(command,"ViewAllUsers"); 
    SendMessageToServer(sock_send, buf, src_ip, dest_ip, command, message, addr_send);

    // Listens for response from server for a message
    FD_SET(sock_send,&readfds);
    select_ret=select(sock_send+1,&readfds,NULL,NULL,NULL);
    if ( (status = ReceiveMessageFromServer(select_ret, sock_send, buf, src_ip, dest_ip, command, message, addr_send) ) == 1){
        if (strcmp(command,"ViewAllUsers") == 0){
        printf("Available Clients\n\n%s\n\n",message);
        }
    }
    else{
    printf("Error in obtaining list of clients from server\n");
    }
}

/* Prints all contacts in the array (if any) and indicates the result of the array 
   traversal whether or not contacts were found by returning an integer.                */
int ViewAllContacts(){
    i = 0;
    int j = 0;
    while (i < (sizeof(users) / sizeof(users[0])) ){
        if ( (strcmp(users[i].ip, "") != 0) ){
            printf("%02d) %s\n",j+1, users[i].name );
            j++;
        }
        i++;
    }
    if (j == 0)
        return -1;  // No Contacts were found
    else
        return 1;   // Contacts were found
}

/* Searches for a contact by username and copys the details of the user into global
   variables for use in other functions and subroutines if found. This returns an integer
   which indicates if the contact was found or not to allow the process to go forward   */
int SearchContacts(char *username){
    i = 0;
    int j = 0;
    while (i < (sizeof(users) / sizeof(users[0])) ){
        if ( (strcmp(users[i].name, username) == 0) ){
            strcpy(chat_username,users[i].name);
            strcpy(chat_ip,users[i].ip);
            j++;
            break;
        }
        i++;
    }
    if (j == 0)
        return -1;  // Contacts was not found
    else
        return 1;   // Contact was found
}

/* Encodes message formatted with delimiters for the server to strip relevant data and
   commands to perform resulting actions. Returns length of message after formatting 
   the message to later use in the sendto function.                                     */

   // Message format: src_ip|dest_ip|command|message
int EncodeMessage(){

    // Message format: src_ip|dest_ip|command|message

    strcpy(buf,user_ip);    // Stores IP of the sender
    strcat(buf,DELIMITER);
    strcat(buf,dest_ip);    // Stores IP of the recipient
    strcat(buf,DELIMITER);
    strcat(buf,command);    // Stores command which directs the server on how to process the message
    strcat(buf,DELIMITER);
    strcat(buf,message);    // Stores message if any 

    return (strlen(user_ip) + strlen(dest_ip) + strlen(command) + strlen(message) + (strlen(DELIMITER) * 3));
}

/* Processes data received and stores the data into the relevant variables for access and use 
   by other functions and methods.                                                      */

   // Message format: src_ip|dest_ip|command|message
void DecodeMessage(){

    strcpy(src_ip,strtok(buf, DELIMITER));      /* Strips the source IP Address */
    strcpy(dest_ip,strtok(NULL, DELIMITER));    /* Strips the destination IP Address */
    strcpy(command,strtok(NULL, DELIMITER));    /* Strips the command sent */
    strcpy(message,strtok(NULL, DELIMITER));       /* Strips the message body sent */
}

/* Prints out the main menu screen */
void MainMenu(char *ip_address){
    // Main Menu
    system("clear");        
    printf ("WhatsApp Clone Main Menu\n\nUsername: %s\nIP: %s\n\n",username,ip_address);
    
    // dynamically prints the groups the user has subscribed to if any
    if (WorkGroup == 1 || FunGroup == 1) {
        printf("Groups Subscribed: ");
        if (WorkGroup == 1) printf("Working Group    ");
        if (FunGroup == 1) printf("Fun Group    ");
    }

    printf ("\n\n[V]iew all online clients on the server\n");
    printf ("[A]dd a client to your contacts\n");
    printf ("[S]end message to a client\n");
    printf ("[G]roups Menu\n\n");
    printf ("[Q]uit \n\n");
}

/* Encodes and send message formatted for the server to process */
void SendMessageToServer(){
    send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
    bytes_sent=sendto(sock_send, buf, send_len, 0,(struct sockaddr *) &addr_send, sizeof(addr_send));
}

/* Processes data received from server and returns status value to indicate success or failure.
   This also calls the DecodeMessage function to strip the data for use in other functions */
int ReceiveMessageFromServer(){
    if (select_ret > 0) {		        /* anything arrive on any socket? */
        int incoming_len=sizeof(addr_send);	/* who sent to us? */
        int recv_msg_size=recvfrom(sock_send,buf,BUF_SIZE,0,(struct sockaddr *)&addr_send,&incoming_len);
        
        if (recv_msg_size > 0){	/* what was sent? */
            buf[recv_msg_size]='\0';
            DecodeMessage(buf, src_ip, dest_ip, command, message);
            return 1;
        }
    }
    return -1;
}

/* Joins a specified group by sending the command variable to the server which is set beforehand */
int JoinGroup(){
    SendMessageToServer(sock_send, buf, src_ip, dest_ip, command, message, addr_send);

    FD_SET(sock_send,&readfds);
    select_ret=select(sock_send+1,&readfds,NULL,NULL,NULL);
    if ( (status = ReceiveMessageFromServer(select_ret, sock_send, buf, src_ip, dest_ip, command, message, addr_send) ) == 1){
        if (strcmp(command,"JoinGroupSuccess") == 0){
            printf("Successfully joined group\n");
            return 1;
        }
    }
    printf("Error - could not join group\n");
    return 0;
}

/* Leaves a specified group by sending the command variable to the server which is set beforehand */
int LeaveGroup(){
    SendMessageToServer(sock_send, buf, src_ip, dest_ip, command, message, addr_send);

    FD_SET(sock_send,&readfds);
    select_ret=select(sock_send+1,&readfds,NULL,NULL,NULL);
    if ( (status = ReceiveMessageFromServer(select_ret, sock_send, buf, src_ip, dest_ip, command, message, addr_send) ) == 1){
        if (strcmp(command,"LeaveGroupSuccess") == 0){
            printf("Successfully left group\n");
            return 1;
        }
    }
    printf("Error - could not leave group\n");
    return 0;
}

/* Sends a broadcast message to the specified group parameter only if the sender is subscribed to
   the group they wish to broadcast to. Otherwise an error message is printed.                    */
void SendGroupBroadcastMessage(char *Group){
    if ((strcmp(Group,"Work") == 0) && (WorkGroup == 1) ){
        system("clear");
        printf("\t\t\tWork Group Chat\n\n\t\tEnter 'QuitChat' to return to main menu\n\n");
        ClientChat(WorkGroupChatFileName, "WorkGroupBroadcast");
        printf("\n\nPress Enter to return to groups menu");
    }
    else if ((strcmp(Group,"Fun") == 0) && (FunGroup == 1) ){
        system("clear");
        printf("\t\t\tFun Group Chat\n\n\t\tEnter 'QuitChat' to return to main menu\n\n");
        ClientChat(FunGroupChatFileName, "FunGroupBroadcast");
        printf("\n\nPress Enter to return to groups menu");
    }
    else {
        printf("You are not subscribed to the %s Group\n",Group);
        printf("You must join the group if you want to send broadcast messages\n");
        printf("Press Enter to return to groups menu");
    }
}    

/* Routine to handle connecting to a client to add as a contact. A user must be added as a contact 
   before any communication can happen between the two users. This has a Timeout value specified for 
   the select function to allow the program to continue if the user has not replied. 
   
   Note that the user that is being connected to has to be on the main menu screen for them 
   to see the message to join for a chat and add as a contact.                                    */
void ConnectToChat(){
    printf("Please enter the username of the person you want to add as a contact:\n");
    fgets (message, 30, stdin);
    message[strlen(message)-1] = '\0';
    strcpy(chat_username,message);                

    strcpy(command,"ConnectToChat");
    strcpy(dest_ip,username);

    SendMessageToServer();
    FD_SET(sock_send,&readfds);

    if (select_ret=select(sock_send+1,&readfds,NULL,NULL,&tv) > 0){ // Waits 10 Seconds for a response from server 
        if ( (status = ReceiveMessageFromServer() ) == 1){
            if (strcmp(command,"ChatAccepted") == 0){
                //strcpy(chat_username,message);
                //strcpy(chat_ip,src_ip);
                
                i = FindEmptySpace();
                if (i != -1) {
                    // Adding new user to contacts to enable chat
                    strcpy(users[i].ip,src_ip);
                    strcpy(users[i].name,chat_username);
                    strcpy(buf,username);
                    strcat(buf,"ChatWith");
                    strcat(buf,chat_username);
                    strcpy(users[i].file_name,buf); // This file will store all chat convos between users

                    
                    printf("Successfully Added New Contact %s at %s\n",users[i].name,users[i].ip);
                }
                else {
                    printf("Error occurred - User was not added\n");
                }
                //ClientChat(); // Start the client chat subroutine
            }

            if (strcmp(command,"ChatDeclined") == 0){
            printf("User declined to chat\n\n");
            }

            if (strcmp(command,"UserNotFound") == 0){
            printf("Error - User not found\n\n");
            }
        }
    } 
    else {
        printf("Timeout exceded. No response received\n");
        printf("Connecting to chat failed\n");
    }
    printf("\nPress Enter to return to main menu...");
}

/* Bootleg Whatsapp Chat Screen which handles sending and receiving messages to and from the server
   in real time to recipient(s). This is used for individual chat between two clients or broadcast 
   messages in group chats. This function accepts the name of the file containing the previous chat
   (if any) and uses this parameter to update the file with the messages received and sent.       */
void ClientChat(char *name_of_file, char *Type){
    ReadMessages(name_of_file); // Prints previous convo before allowing you to send messages
    
    while (1){ 
        printf("You: ");
        fflush(stdout);

        FD_SET(sock_send,&readfds);	/* Server Messages  */
        FD_SET(0, &readfds);         /* Input from Stdin */
        select_ret=select(sock_send+1, &readfds, NULL, NULL, NULL);
        
        if(FD_ISSET(sock_send,&readfds)){ // New message from client through the server
            if ( (status = ReceiveMessageFromServer() ) == 1){
                if (strcmp(command,"ClientChatMessage") == 0){
                    i = ClientLookUp(src_ip, "");   // Incoming message might not be from current contact being talked to
                    WriteMessageToFile(users[i].file_name, users[i].name, message); // Stores message into file
                    
                    if (strcmp(users[i].ip,chat_ip) == 0){    // Only prints to screen if the message is from the current chat
                        printf("\33[2K\r%s: %s\n",users[i].name,message); // Erases current line and prints incoming message
                    }                
                }
                
                // Processes Group Broadcast Messages
                if (strcmp(command,"WorkGroupBroadcast") == 0){
                    WriteMessageToFile(WorkGroupChatFileName, dest_ip, message); // Stores message into file
                    if (strcmp(Type,"WorkGroupBroadcast") == 0){
                        printf("\33[2K\r%s: %s\n",dest_ip,message); // Erases current line and prints incoming message
                    }                        
                }

                if (strcmp(command,"FunGroupBroadcast") == 0){
                    WriteMessageToFile(FunGroupChatFileName, dest_ip, message); // Stores message into file
                    if (strcmp(Type,"FunGroupBroadcast") == 0){
                        printf("\33[2K\r%s: %s\n",dest_ip,message); // Erases current line and prints incoming message
                    }                        
                }
            }
        }

        if(FD_ISSET(0, &readfds)){ // input from Stdin to be sent to other client
            fgets (message, 200, stdin);
            message[strlen(message)-1] = '\0'; 

            if(message[0] != '\0'){ // Doesn't send if the user enters nothing
                if (strcmp(message,"QuitChat") == 0){ // Terminates chat. This message 'QuitChat' is
                    memset(chat_ip,0,sizeof(chat_ip)); // not written to the chat file or sent to server.
                    break;                            
                }
                
                if (strcmp(Type,"WorkGroupBroadcast") == 0){
                    strcpy(command,"WorkGroupBroadcast");
                    strcpy(dest_ip,username);
                }

                if (strcmp(Type,"FunGroupBroadcast") == 0){
                    strcpy(command,"FunGroupBroadcast");
                    strcpy(dest_ip,username);
                }

                if (strcmp(Type,"ClientChatMessage") == 0){
                    strcpy(command,"ClientChatMessage");
                    strcpy(dest_ip,chat_ip);
                }
                
                send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                bytes_sent=sendto(sock_send, buf, send_len, 0,(struct sockaddr *) &addr_send, sizeof(addr_send));
                
                WriteMessageToFile(name_of_file, "You", message); // Updates chat file after sending message             
            }
        }
    }
}

/* Looks up a contact by username or IP address and returns the index location of the user in the array
   if found in the array. Otherwise a negative integer is returned to indicate the contact was not found */
int ClientLookUp(char *dest_ip, char *username){
    int i = 0;
    if (strcmp(dest_ip,"") != 0){
        while (i < (sizeof(users) / sizeof(users[0])) ){
            if (strcmp(users[i].ip, dest_ip) == 0){
                return i;
            }
            i++;
        }
        return -1;
    }
    else{
        if (strcmp(username,"") != 0){
            while (i < (sizeof(users) / sizeof(users[0])) ){
                if (strcmp(users[i].name, username) == 0){
                    return i;
                }
                i++;
            }
            return -1;
            }
    }
    return -1;    
}

/* Looks for an empty location in the array and returns the index location that was found empty. This is
   used when adding a new contact to the user array.                                                     */
int FindEmptySpace(){
    int i = 0;
    while (i < (sizeof(users) / sizeof(users[0])) ){
        if (strcmp(users[i].ip, "") == 0){
            return i;
        }
        i++;
    }
    return -1;
}

/* Prints a specified file to screen and returns whether the file was found or not. This is used to print
   a previous chat file before allowing the user to send messages in the chat screen.                    */
int ReadMessages(char *name_of_file){
    if ((fp = fopen(name_of_file,"r")) != NULL){
        while (fgets(buf, sizeof(buf), fp) != NULL) {
        printf("%s",buf);
        }
        fclose(fp);
        return 1;   // File found
    }
    return -1; // File not found    
}

/* Stores the message sender and message received to the specified chat file. This is used to write group 
   broadcast messages or messages from a contact to their respective chat file.                          */
void WriteMessageToFile(char *name_of_file, char *sender, char *sender_message){
    fp=fopen(name_of_file, "a");
    fprintf(fp,"%s: %s\n",sender,sender_message);
    fclose(fp);
}
