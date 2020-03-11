#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h> //inet_addr

#define MESSAGE_BUFFER 1024
#define SERVER_NAME 7

typedef struct {
    char* src;
    char* dest;
    int src_socket;
    int dest_socket;
} thread_data;

    thread_data data[5];

void *connection_handler(void * add)
{

   printf("check 1\n");
    //Get the socket descriptor
    int c = *(int*)add;
    printf("check 2\n");
    int f=0,dest;
    int response,i,fd=0;
    char client_message[2000];
    for(i=0;i<5;i++)
    {
      printf("check 3\n");
      if(strcmp(data[i].src, data[c].dest)==0)
      {
        dest=i;
        break;
      }
    }
    memset(client_message, '\0' ,2000);
    strncpy(client_message, "wait", 2000);
    printf("check 4\n");
    send(data[c].src_socket, client_message, strlen(client_message),0);
    printf("check 5\n");
    while(true)
    {
      if(data[dest].src_socket!=-5)
        break;

    }
    data[c].dest_socket=data[dest].src_socket;

    printf("check 6\ndest_socket: %d\n",data[c].dest_socket);
    memset(client_message, '\0' ,2000);
    strncpy(client_message, "type", 2000);
    send(data[c].src_socket, client_message, strlen(client_message),0);
    printf("check 5\n");
    while(true)
    {
        printf("check 6\n");
        memset(client_message, '\0' ,2000);
        response = recv(data[c].src_socket, client_message, 2000, 0);
        if (response == -1) {
          perror("recv() failed: ");
          break;
        } else if (response == 0) {
              printf("\nPeer disconnected\n");
              data[c].src_socket=-5;
              data[dest].src_socket=-5;
              close(data[c].src_socket);
              close(data[c].dest_socket);
              break;
        } else {

              printf("%s\n",client_message);
              send(data[c].dest_socket , client_message , strlen(client_message),0);
          }
          memset(client_message, '\0' ,2000);
        response = recv(data[c].dest_socket, client_message, 2000, 0);
        if (response == -1) {
          perror("recv() failed: ");
          break;
        } else if (response == 0) {
              printf("\nPeer disconnected\n");
              data[c].src_socket=-5;
              data[dest].src_socket=-5;
              close(data[c].src_socket);
              close(data[c].dest_socket);
              break;
        } else {

              printf("%s\n",client_message);
              send(data[c].src_socket , client_message , strlen(client_message),0);
          }

        
        
    }

    pthread_exit(NULL); 
    return NULL;
}
int main() 
{
    data[0].src="tarang";
    data[1].src="abhi";
    data[2].src="raju";
    data[3].src="ritz";
    data[4].src="dipu";
    data[0].src_socket=-5;
    data[1].src_socket=-5;
    data[2].src_socket=-5;
    data[3].src_socket=-5;
    data[4].src_socket=-5;

    int k,c=0,opt=1,flag=0;
    char buf[1024];
    int *new_sock;
    struct sockaddr_in server, client;
    int fd, fd1;
    socklen_t len;
    pthread_t thread;
        memset(&server,0,sizeof(server));
        memset(&client,0,sizeof(client));

        fd=socket(AF_INET,SOCK_STREAM,0);
        printf("fd : %d\n",fd);
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
        data[0].src_socket=fd+1;

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

        while(fd1 = accept(fd, (struct sockaddr *)&client, &len) )
       {
        puts("Connection accepted");
    //    printf("fd1 : %d\n",fd1);
        memset(buf,'\0',sizeof(buf));
        k=recv(fd1, buf, sizeof(buf),0);
        if(k==-1)
        {
          printf("Error in receiving");
          exit(1);
        }
        for(int i=0;i<5;i++)
        {
          if(strcmp(data[i].src,buf)==0)
          {
            c=i;
            break;
          }
        }
        data[c].src_socket=fd1;
        printf("\nSocket fd: %d\n",data[c].src_socket);
        printf("\nUsername: %s\n",data[c].src);
        memset(buf,'\0',sizeof(buf));
        k=recv(fd1, buf, sizeof(buf),0);
        if(k==-1)
        {
          printf("Error in receiving");
          exit(1);
        }
        data[c].dest=buf;
        printf("\nDestname: %s\n",data[c].dest);

      //  printf("\nUser %d :\nFD = %d\nUsername: %s\nDestination: %s",c,fd1,data[c].src,data[c].dest);


        pthread_t sniffer_thread;



        if( pthread_create( &sniffer_thread , NULL ,  connection_handler ,(void*) &c) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL); // was commented before
        puts("Handler assigned");
      }

        if(fd1==-1)
        {
            printf("Error in temporary socket creation");
            exit(1);
        }
        


        

    // Create data struct for new thread
    
    return 0;
}



