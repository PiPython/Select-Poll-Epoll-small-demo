#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<poll.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/fcntl.h>


#define MAXNFDS 1024
//初始化服务端的监听端口
int initserver(int port);

int main(int argc,char * argv[]){
	if(argc != 2){
		printf("usage:./server port\n");
		return -1;
	}
	
	int listensock = initserver(atoi(argv[1]));
	printf("listensock = %d\n", listensock);
	
	if (listensock < 0)
	{
		printf("initserver() failed.\n"); return -1;
	}
	
	int maxfd;
	struct pollfd fds[MAXNFDS];
	
	for(int ii=0;ii<MAXNFDS;++ii){
		fds[ii].fd = -1;
	}
	
	fds[listensock].fd = listensock;
	fds[listensock].events = POLLIN;
	maxfd = listensock;
	
	
	while(1){
		int infds = poll(fds,maxfd+1,5000);
		
		if(infds<0){
			printf("poll() failed.\n");
			break;
		}
		
		if(infds == 0){
			printf("poll() timeout.\n");
			continue;
		}
		
		
	for(int eventfd = 0;eventfd <= maxfd;++eventfd){
            if(fds[eventfd].fd <0) continue;
            
            if((fds[eventfd].revents & POLLIN) == 0)	continue;
            
            fds[eventfd].revents = 0;
            
            if(eventfd == listensock){
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
                if(clientsock<0){
                    printf("accept() failed.\n");
                    continue;
                }

                printf("client(socket = %d) connected OK.\n",clientsock);

                fds[clientsock].fd = clientsock;
		fds[clientsock].events = POLLIN;
		fds[clientsock].revents = 0;
                if(maxfd < clientsock)
                    maxfd = clientsock;
            }
            else{
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));

                ssize_t isize = read(eventfd,buffer,sizeof(buffer));

                if(isize <= 0){
                    printf("client(socket = %d) disconnected.\n",eventfd);
                    close(eventfd);
                    fds[eventfd].fd = -1;
                    if(eventfd == maxfd){
                        for(int ii=maxfd ; ii>0; ii--){
                            if(fds[maxfd].fd >= 0){
                                maxfd = ii;
                                break;
                            }
                        }
                    }
                    continue;
                }

                printf("recv(eventfd=%d,size=%ld):%s\n",eventfd,isize,buffer);

                write(eventfd,buffer,strlen(buffer));
            }
		}
	}	
}


int initserver(int port){
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		printf("socket() failed.\n");
		return -1;
	}
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	
	if(bind(sock,(struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		printf("bind() failed.\n");close(sock);return -1;
	}
	
	if(listen(sock,5)!=0){
		printf("listen() failed.\n");close(sock);return -1;
	}
	
	
	return sock;
}
