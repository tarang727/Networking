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


int fd;
// Get message from stdin and send to server
void *receive(void *vargp) 
{
    int response;
    char message[1024];
    
    while(true) {
        memset(message, '\0', sizeof(message));
        response = recv(fd, message, 1024, 0);
        if (response == -1) 
        {
          perror("recv() failed: ");
          break;
        } 
        else if(response == 0) 
        {
              printf("\nPeer disconnected\n");
              break;
        } 
        else 
        {
              printf("\n%s\n", message);

          }
    }
}


void *send_message(void *vargp) {
  char message[1024];
  while (true){
      memset(message,'\0',sizeof(message));
      fgets(message, 1024, stdin);
      if (strncmp(message, "quit", 4) == 0) {
        printf("\nBye....\n\n");
        exit(0);
      }
      send(fd, message, strlen(message), 0);
      printf("\n");
  }
}

int main() {
    int k;
    socklen_t len;
    struct sockaddr_in client;
    memset(&client,0,sizeof(client));
    char username[1024];
    pthread_t thread[2];
    
    fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1)
    {
        printf("Error in socket creation");
        exit(1);
    }

    client.sin_family=AF_INET;
    client.sin_addr.s_addr=inet_addr("127.0.0.1");
    client.sin_port=htons(8080);
    len=sizeof(client);

    k=connect(fd,(struct sockaddr*)&client, len);
    if(k==-1)
    {
        printf("Error in connecting to server");
        exit(1);
    }

    printf("Connection established.\n");

    printf("Enter your user name: ");
    fgets(username, 1024, stdin);
    username[strlen(username) - 1] = '\0'; // Remove newline char from end of string
    k=send(fd, username, strlen(username),0);
        if(k==-1)
        {
            printf("Error in sending");
            exit(1);
        }

    pthread_create(&thread[0], NULL, receive, NULL);
    pthread_create(&thread[1], NULL, send_message, NULL);

    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
    printf("Bye....\n");
    close(fd);
    return 0;
}