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
#define USERNAME_BUFFER 1024

typedef struct {
    char* prompt;
    int socket;
} thread_data;


// Get message from stdin and send to server
void * send_message(char prompt[USERNAME_BUFFER+4], int socket_fd, struct sockaddr_in *address) {
  printf("%s", prompt);
  char message[MESSAGE_BUFFER];
  char final_message[MESSAGE_BUFFER+USERNAME_BUFFER+1];
  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
      memset(final_message,0,strlen(final_message)); // Clear final message buffer
      strcat(final_message, prompt);
      strcat(final_message, message);
      printf("\n%s", prompt);
      if (strncmp(message, "/quit", 5) == 0) {
        printf("Closing connection...\n");
        exit(0);
      }
      send(socket_fd, final_message, strlen(final_message)+1, 0);
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
          fprintf(stderr, "recv() failed: %s\n", strerror(errno));
          break;
        } else if (response == 0) {
              printf("\nPeer disconnected\n");
              break;
        } else {
              printf("\n%s", message);
              printf("%s", prompt);
              fflush(stdout); // Make sure "User>" gets printed
          }
    }
}

int main() {
    int k,fd;
    socklen_t len;
    struct sockaddr_in client;
    char prompt[USERNAME_BUFFER+4];
    char username[USERNAME_BUFFER];
    pthread_t thread;

    // Get user handle
    printf("Enter your user name: ");
    fgets(username, USERNAME_BUFFER, stdin);
    username[strlen(username) - 1] = 0; // Remove newline char from end of string
    strcpy(prompt, username);
    strcat(prompt, "> ");

    memset(&client,0,sizeof(client));

    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1)
    {
        printf("Error in socket creation");
        exit(1);
    }

    client.sin_family=AF_INET;
    client.sin_addr.s_addr=inet_addr("10.2.95.53");
    client.sin_port=htons(8080);

    k=connect(fd,(struct sockaddr*)&client,sizeof(client));
    if(k==-1)
    {
        printf("Error in connecting to server");
        exit(1);
    }

    printf("Connection established. You may start typing...\n");

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