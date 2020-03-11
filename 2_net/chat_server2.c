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
#define SERVER_NAME 7

typedef struct {
    char* prompt;
    int socket;
} thread_data;


// Get message from stdin and send to client
void * send_message(char prompt[SERVER_NAME+4], int new_socket_fd) {
  printf("%s", prompt);
  char message[MESSAGE_BUFFER];
  char final_message[MESSAGE_BUFFER+SERVER_NAME+1];
  while (fgets(message, MESSAGE_BUFFER, stdin) != NULL) {
      memset(final_message,0,strlen(final_message)); // Clear final message buffer
      strcat(final_message, prompt);
      strcat(final_message, message);
      printf("\n%s", prompt);
      if (strncmp(message, "/quit", 5) == 0) {
        printf("Closing connection...\n");
        exit(0);
      }
      send(new_socket_fd, final_message, strlen(final_message)+1, 0);
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
    int k,opt=1;
    struct sockaddr_in server, client;
    char* prompt = "server>";
    int fd, fd1;
    socklen_t len;
    pthread_t thread;
        memset(&server,0,sizeof(server));
        memset(&client,0,sizeof(client));

        fd=socket(AF_INET,SOCK_STREAM,0);
        if(fd==-1)
        {
            printf("Error in socket creation");
            exit(1);
        }

        k=setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if(k==-1)
        {
            printf("Error in setsockopt");
            exit(1);
        }

        server.sin_family=AF_INET;
        server.sin_addr.s_addr=INADDR_ANY;
        server.sin_port=htons(8080);
        k=bind(fd,(struct sockaddr*)&server, sizeof(server));
        if(k==-1)
        {
            printf("Error in binding");
            exit(1);
        }
        k=listen(fd,20);
        if(k==-1)
        {
            printf("Error in listening");
            exit(1);
        }
        printf("Waiting for incoming connection...\n");

        len=sizeof(client);
        fd1=accept(fd,(struct sockaddr*)&client,&len);
        if(fd1==-1)
        {
            printf("Error in temporary socket creation");
            exit(1);
        }

        printf("Connection established. You may start typing...\n");

    // Create data struct for new thread
    thread_data data;
    data.prompt = prompt;
    data.socket = fd1;

    // Create new thread to receive messages
    pthread_create(&thread, NULL, receive, (void *) &data);

    // Send message
    send_message(prompt, fd1);

    // Close sockets and kill thread
    close(fd1);
    close(fd);
    pthread_exit(NULL);
    return 0;
}