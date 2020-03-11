#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "netdb.h"
#include "netinet/in.h"
#include "pthread.h"
#include "errno.h"
#include "unistd.h"
#include <arpa/inet.h> //inet_addr

#define MESSAGE_BUFFER 1024
#define USERNAME_BUFFER 100

typedef struct {
    char* prompt;
    int socket;
} thread_data;


// Get message from stdin and send to server
void * send_message(char prompt[USERNAME_BUFFER+4], int socket_fd, struct sockaddr_in *address) {
  printf("%s", prompt);
  char message[MESSAGE_BUFFER];
  char final_message[MESSAGE_BUFFER+USERNAME_BUFFER+1];
  while (true){//fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
      memset(message,'\0',strlen(message));
      fgets(message, MESSAGE_BUFFER, stdin);
      memset(final_message,'\0',strlen(final_message)); // Clear final message buffer
      strcat(final_message, prompt);
      strcat(final_message, message);
      printf("\n%s", prompt);
      if (strncmp(message, "quit", 4) == 0) {
        printf("Closing connection...\n");
        exit(0);
      }
      send(socket_fd, final_message, strlen(final_message), 0);
  }
}

void * receive(void * threadData) {
    int socket_fd, response;
    char message[MESSAGE_BUFFER];
    thread_data* pData = (thread_data*)threadData;
    socket_fd = pData->socket;
    char* prompt = pData->prompt;
    memset(message, 0, MESSAGE_BUFFER); // Clear message buffer

    // Print received message
    while(true) {
        response = recv(socket_fd, message, MESSAGE_BUFFER, 0);
        if (response == -1) {
          perror("recv() failed: ");
          break;
        } else if(response==0){
          printf("Peer disconnected...");
          exit(0);

        } else{
            
              printf("\n%s\n", message);
              printf("%s\n", prompt);
            
          }
    }
}

int main() {
    int k,fd;
    socklen_t len;
    struct sockaddr_in client;
    char prompt[USERNAME_BUFFER+4];
    char username[USERNAME_BUFFER];
    char destname[USERNAME_BUFFER];

    pthread_t thread;



    memset(&client,0,sizeof(client));

    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1)
    {
        printf("Error in socket creation");
        exit(1);
    }

    client.sin_family=AF_INET;
    client.sin_addr.s_addr=inet_addr("127.0.0.1");
    client.sin_port=htons(8080);

    k=connect(fd,(struct sockaddr*)&client,sizeof(client));
    if(k==-1)
    {
        printf("Error in connecting to server");
        exit(1);
    }

    printf("Connection established. You may start typing...\n");

    printf("Enter your user name: ");
    fgets(username, USERNAME_BUFFER, stdin);
    username[strlen(username) - 1] = '\0'; // Remove newline char from end of string
    k=send(fd, username, strlen(username),0);
        if(k==-1)
        {
            printf("Error in sending");
            exit(1);
        }
    printf("Enter username of person to talk to: ");
    fgets(destname, USERNAME_BUFFER, stdin);
    destname[strlen(destname) - 1] = '\0'; // Remove newline char from end of string
    k=send(fd, destname, strlen(destname),0);
        if(k==-1)
        {
            printf("Error in sending");
            exit(1);
        }
    
    


    strcpy(prompt, username);
    strcat(prompt, "> ");

    // Create data struct for new thread
    thread_data data;
    data.prompt = prompt;
    data.socket = fd;

    // Create new thread to receive messages
    pthread_create(&thread, NULL, receive, (void *) &data);

    // Send message
    send_message(prompt, fd, &client);

    // Close socket and kill thread
    close(fd);
    pthread_exit(NULL);
    return 0;
}