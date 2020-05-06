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
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE	1024
#define LISTEN_PORT	60000
#define DELIMITER   "%^&*"

typedef struct {
   char     ip[20];
   char     name[30];
   struct   sockaddr_in socket;
   int      WorkGroup;
   int      FunGroup;
} Userprofile;

Userprofile users[20] = {0}; // Stores list of active users' IP addresses and names
int FindEmptySpace();
void ViewAllClients(char *message);

/* Encodes message formatted with delimiters for the server to strip relevant data and
   commands to perform resulting actions. Returns length of message after formatting 
   the message to later use in the sendto function.                                     */

   // Message format: src_ip|dest_ip|command|message
int EncodeMessage(char *buf, char *src_ip, char *dest_ip, char *command, char *body){
    
    strcpy(buf,src_ip);
    strcat(buf,DELIMITER);
    strcat(buf,dest_ip);
    strcat(buf,DELIMITER);
    strcat(buf,command);
    strcat(buf,DELIMITER);
    strcat(buf,body); 

    return (strlen(src_ip) + strlen(dest_ip) + strlen(command) + strlen(body) + (strlen(DELIMITER) * 3));
}

/* Processes data received and stores the data into the relevant variables for access and use 
   by other functions and methods.                                                      */

   // Message format: src_ip|dest_ip|command|message
void DecodeMessage(char *buf, char *src_ip, char *dest_ip, char *command, char *body){
    strcpy(src_ip,strtok(buf, DELIMITER));      /* Strips the source IP Address */
    strcpy(dest_ip,strtok(NULL, DELIMITER));    /* Strips the destination IP Address */
    strcpy(command,strtok(NULL, DELIMITER));    /* Strips the command sent */
    strcpy(body,strtok(NULL, DELIMITER));       /* Strips the message body sent */
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

int main(int argc, char *argv[]){
    int			sock_recv, client_socket[15] = {0};
    int         max_clients = 15, max_sd, sd;
    struct      sockaddr_in	my_addr;
    int			i, status = 0;
    fd_set	    readfds;
    int			incoming_len, send_len, bytes_sent;
    struct      sockaddr_in	remote_addr;
    int			recv_msg_size;
    char		buf[BUF_SIZE];
    int			select_ret;

    char src_ip[30];
    char dest_ip[30];
    char message[1024];
    char command[30];


            /* create socket for receiving */
    sock_recv=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sock_recv < 0) { printf("socket() failed\n"); exit(0); }
        
        /* make local address structure */
    memset(&my_addr, 0, sizeof (my_addr));	/* zero out structure */
    my_addr.sin_family = AF_INET;	/* address family */
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* current machine IP */
    my_addr.sin_port = htons((unsigned short)LISTEN_PORT);
        
        /* bind socket to the local address */
    i=bind(sock_recv, (struct sockaddr *) &my_addr, sizeof (my_addr));
    if (i < 0) { printf("bind() failed\n"); exit(0); }
    
        /* listen ... */
    printf("Server started on port %d\n\n",LISTEN_PORT);
    
    while (1){
        FD_ZERO(&readfds);		        /* zero out socket set */
        FD_SET(sock_recv,&readfds);	/* add socket to listen to */
        
        printf("Listening for connections...\n");
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        select_ret=select(sock_recv+1,&readfds,NULL,NULL,NULL);
          
        /* Incoming connection from a client to be processed */
        if (FD_ISSET(sock_recv, &readfds)) 
        {   
            if (select_ret > 0){		/* anything arrive on any socket? */
            incoming_len=sizeof(remote_addr);	/* who sent to us? */
            recv_msg_size=recvfrom(sock_recv,buf,BUF_SIZE,0,(struct sockaddr *)&remote_addr,&incoming_len);

                if (recv_msg_size > 0){	/* what was sent? */
                    buf[recv_msg_size]='\0';
                    DecodeMessage(buf, src_ip, dest_ip, command, message);
                    
                    printf("Received Message from %s Message Size: %d\n",inet_ntoa(remote_addr.sin_addr),recv_msg_size);
                    printf("Message Details\n");
                    printf("Source Address: %s\n",src_ip);
                    printf("Destination Address: %s\n",dest_ip);
                    printf("Command: %s\n\n",command);
                    printf("Message: %s\n\n",message);
                    
                    if (strcmp(command,"NewUser") == 0){
                        
                        i = FindEmptySpace();
                        if (i != -1) {
                            strcpy(command,"NewUserSuccess");
                            
                            // Initialization of new user in array
                            strcpy(users[i].ip,src_ip);
                            strcpy(users[i].name,message);
                            users[i].WorkGroup = 0;
                            users[i].FunGroup = 0;
                            users[i].socket = remote_addr;
                            
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("Bytes Sent: %d\n",bytes_sent);
                            
                            printf("Added New User %s at %s\n",users[i].name,users[i].ip);
                        }
                        else {
                            printf("Error occurred - User was not added\n");
                        }
                    }

                    else if (strcmp(command,"ViewAllUsers") == 0){
                        ViewAllClients(message);
                        strcpy(command,"ViewAllUsers");
                        send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                        bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
            
                        printf("View All Users Bytes Sent: %d\n",bytes_sent);
                    }

                    else if (strcmp(command,"ConnectToChat") == 0){
                        i = ClientLookUp("",message);
                        if (i != -1){
                            strcpy(command,"ConnectToChat");
                            strcpy(message,dest_ip);
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("%s attempting to connect to %s\n",message,users[i].name);
                            printf("Bytes Sent: %d\n",bytes_sent);
                        }
                        else{
                            strcpy(command,"UserNotFound");
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
                            printf("Error - User not Found\n");
                        }
                    }

                    else if (strcmp(command,"ChatAccepted") == 0){
                        i = ClientLookUp(dest_ip,"");
                        if (i != -1){
                            strcpy(command,"ChatAccepted");                            
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("Bytes Sent: %d\n",bytes_sent);
                        }
                        else{
                            printf("Error - User not Found\n");
                        }
                    }

                    else if (strcmp(command,"ClientChatMessage") == 0){
                        i = ClientLookUp(dest_ip,"");
                        if (i != -1){
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("Bytes Sent: %d\n",bytes_sent);
                        }
                        else{
                            printf("Error - User not Found\n");
                        }
                    }

                    else if (strcmp(command,"JoinGroup") == 0){
                        i = ClientLookUp(src_ip,"");
                        if (i != -1){
                            if (strcmp(message, "WorkGroup") == 0){
                                users[i].WorkGroup = 1;
                            }
                            if (strcmp(message, "FunGroup") == 0){
                                users[i].FunGroup = 1;
                            }                            
                            strcpy(command,"JoinGroupSuccess");
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("JoinGroupSuccess Bytes Sent: %d\n",bytes_sent);
                        }
                        else{
                            strcpy(command,"JoinGroupError");
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
                            printf("JoinGroupError Bytes Sent: %d\n",bytes_sent);
                            printf("Error - User not Found\n");
                        }
                    }

                    else if (strcmp(command,"LeaveGroup") == 0){
                        i = ClientLookUp(src_ip,"");
                        if (i != -1){
                            if (strcmp(message, "WorkGroup") == 0){
                                users[i].WorkGroup = 0;
                            }
                            if (strcmp(message, "FunGroup") == 0){
                                users[i].FunGroup = 0;
                            }                            
                            strcpy(command,"LeaveGroupSuccess");
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                            printf("LeaveGroupSuccess Bytes Sent: %d\n",bytes_sent);
                        }
                        else{
                            strcpy(command,"LeaveGroupError");
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message); 
                            bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &remote_addr, sizeof(remote_addr));
                            printf("LeaveGroupError Bytes Sent: %d\n",bytes_sent);
                            printf("Error - User not Found\n");
                        }
                    }

                    else if (strcmp(command,"WorkGroupBroadcast") == 0){
                        i = ClientLookUp(src_ip,"");
                        strcpy(dest_ip,users[i].name);
                        printf("Started Broadcasting Message to Work Group\n");
                        i = 0;
                        while (i < (sizeof(users) / sizeof(users[0])) ){
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message);
                            if ( (strcmp(users[i].ip, "") != 0) && (users[i].WorkGroup == 1) && (strcmp(users[i].ip,src_ip) != 0)  ){
                                bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                                printf("Sent Broadcast to %s at %s\n",users[i].name ,users[i].ip);
                            }
                            i++;
                        }
                        printf("Finished Broadcasting Message to Work Group\n");
                    }

                    else if (strcmp(command,"FunGroupBroadcast") == 0){
                        i = ClientLookUp(src_ip,"");
                        strcpy(dest_ip,users[i].name);
                        printf("Started Broadcasting Message to Fun Group\n");
                        i = 0;
                        while (i < (sizeof(users) / sizeof(users[0])) ){
                            send_len = EncodeMessage(buf, src_ip, dest_ip, command, message);
                            if ( (strcmp(users[i].ip, "") != 0) && (users[i].WorkGroup == 1) && (strcmp(users[i].ip,src_ip) != 0)  ){
                                bytes_sent=sendto(sock_recv, buf, send_len, 0,(struct sockaddr *) &users[i].socket, sizeof(users[i].socket));
                                printf("Sent Broadcast to %s at %s\n",users[i].name ,users[i].ip);
                            }
                            i++;
                        }
                        printf("Finished Broadcasting Message to Fun Group\n");
                    }
                
                } // End of (recv_msg_size > 0)
            } // End of if (select_ret > 0)
        } // End of FD_ISSET 
    } // End of while loop
    close(sock_recv);
}

/* Returns index of an empty location in the list of users array */
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

/* Prints a list of users currently available online */
void ViewAllClients(char *message){
    int i = 0;
    memset(message,0,sizeof(message));
    while (i < (sizeof(users) / sizeof(users[0])) ){
        if (strcmp(users[i].ip, "") != 0){
            sprintf(&message[strlen(message)], "%02d) %s\n",i+1,users[i].name);
        }
        i++;
    }
    printf("Available Users: %s",message);
}
