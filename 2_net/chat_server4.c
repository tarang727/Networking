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

typedef struct {
    char* src;
    int socket;
} user_data;

 user_data data[5];


int main()
{
    data[0].src="tarang";
    data[1].src="abhi";
    data[2].src="ravi";
    data[3].src="rahul";
    data[4].src="vijay";

    struct sockaddr_in server, client;
    socklen_t len;
    int i,j,k,opt = 1;
    int server_fd, fdmax, new_fd;
    char buffer[1024] = {'\0'};
    char temp[1024]={'\0'};
    char final_message[1024] = {'\0'};

    memset(&server,0,sizeof(server));
    memset(&client,0,sizeof(client));

    fd_set master;
    fd_set readfds;
    FD_ZERO(&master);
    FD_ZERO(&readfds);

    for (i = 0; i < 5; i++)
    {
        data[i].socket = -5;
    }


    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd==-1)
    {
        perror("Error in socket creation :");
        exit(1);
    }

    k=setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(k==-1)
    {
        perror("Error in setsockopt :");
        exit(1);
    }

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(8080);

    k=bind(server_fd,(struct sockaddr*)&server, sizeof(server));
    if(k==-1)
    {
        perror("Error in binding : ");
        exit(1);
    }
    k=listen(server_fd,50);
    if(k==-1)
    {
        perror("Error in listening : ");
        exit(1);
    }
    printf("Waiting for incoming connections...\n");

    len=sizeof(client);

    FD_SET(server_fd,&master);
    fdmax=server_fd;

    while(true)
    {
        readfds = master;
        

        k = select(fdmax+1, &readfds, NULL, NULL, NULL);

        if (k==-1)
            {
                perror("select() error: ");
                exit(1);     
            }

        for(i=0;i<=fdmax;i++)
        {
            if (FD_ISSET(i, &readfds))
                {
                    if(i==server_fd)
                    {
                        // handle new connections
                        new_fd = accept(server_fd,(struct sockaddr *)&client, &len);
                        if (new_fd==-1) 
                        {
                            perror("Faliure to accept : ");
                            exit(1);
                        }

                        FD_SET(new_fd, &master);
                        if(new_fd>fdmax)
                            fdmax=new_fd;
                        printf("******************************************************************************\n");
                        printf("New user joined with socket fd : %d\n", new_fd);
                        memset(buffer,'\0', 1024);
                        k=recv(new_fd, buffer, 1024,0);
                        if(k==-1)
                        {
                          printf("Error in receiving");
                          exit(1);
                        }
                        buffer[k]='\0';
                        for(j=0;j<5;j++)
                        {
                          if(strcmp(data[j].src,buffer)==0)
                          {
                            data[j].socket=new_fd;
                            printf("User fd added to database\n");
                            break;
                          }
                        }
                        memset(temp, '\0', 1024);
                        strncpy(temp, "You may start typing now...\n", 1024);
                        k=send(new_fd, temp, strlen(temp),0);
                        printf("Username : %s\n", data[j].src);
                        printf("List of clients added :\n");

                        for (j=0; j<5; j++)
                        {
                            if (data[j].socket!=-5)
                            {
                                printf("Username of client : %s\tSocket fd : %d\tPresent status: Online\n", data[j].src, data[j].socket);
                            }
                        }

                        printf("******************************************************************************\n");
                    }

                    else
                    {
                        int s,d,f=0;
                        memset(buffer,'\0', 1024);
                        // handle data from a client
                        k = recv(i, buffer, 1024,0);
                        
                        for(j=0;j<5;j++)
                        {
                            if(data[j].socket==i)
                            {
                                s=j;
                                break;
                            }
                        }                       
                        if (k <= 0)
                        {
                            if(k==0)
                            {
                            /* somebody disconnected */
                            printf("Username of client : %s\tSocket fd : %d\tPresent status: Offline\n", data[s].src, data[s].socket);
                            printf("******************************************************************************\n");
                            }
                            else
                            {
                                perror("Recv failed : ");

                            }

                            close(i); // socket closed
                            FD_CLR(i, &master); // remove from master set
                            data[s].socket=-5;

                        }
                        else
                        {
                            memset(temp, '\0', 1024);
                            char dest[20]={'\0'};
                            int n=strcspn(buffer, " ");
                            strncpy(dest, buffer, n);
                            strncpy(temp, buffer+n, 1024);

                            for(j=0;j<5;j++)
                            {
                                if(strcmp(dest, data[j].src)==0)
                                {
                                    d=j;
                                    f=1;
                                    break;
                                }
                            }
                            if(f==0)
                            {
                                memset(temp, '\0', sizeof(temp));
                                strcpy(temp, "User doesn't exist. Send valid username.\n");
                                send(i, temp, strlen(temp), 0);
                                continue;
                            }
                            if(data[d].socket==-5)
                            {
                                memset(temp, '\0', sizeof(temp));
                                strcpy(temp, "User not yet online. Please wait...\n");
                                send(i, temp, strlen(temp), 0);
                                continue;
                            }
                          
                            memset(final_message, '\0', 1024);
                            strcat(final_message, data[s].src);
                            strcat(final_message, ":");
                            strcat(final_message, temp);
                            send(data[d].socket, final_message, strlen(final_message), 0);
                        }
                }
            }
        }               
    }

    return 0;
}
