#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<poll.h>

#include<sys/epoll.h>

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
	
	int epfd = epoll_create(1);
	
	struct epoll_event e;
	e.events = EPOLLIN;
	e.data.fd = listensock;
	
	epoll_ctl(epfd,EPOLL_CTL_ADD,listensock,&e);
	
	
	while(1){
	
	struct epoll_event ret[1024];
		int infds = epoll_wait(epfd,ret,1024,-1);
		
		if(infds<0){
			printf("poll() failed.\n");
			break;
		}
		
		if(infds == 0){
			printf("poll() timeout.\n");
			continue;
		}
		
		
	for(int ii = 0;ii <= infds;++ii){
	
            if(ret[ii].data.fd == listensock){
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
                if(clientsock<0){
                    printf("accept() failed.\n");
                    continue;
                }

                printf("client(socket = %d) connected OK.\n",clientsock);

               struct epoll_event e;
		e.events = EPOLLIN;
		e.data.fd = clientsock;
	
		epoll_ctl(epfd,EPOLL_CTL_ADD,clientsock,&e);
            }
            else{
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));

                ssize_t isize = read(ret[ii].data.fd,buffer,sizeof(buffer));

                if(isize <= 0){
                    printf("client(socket = %d) disconnected.\n",ret[ii].data.fd);
                    close(ret[ii].data.fd);
                    epoll_ctl(epfd,EPOLL_CTL_DEL,ret[ii].data.fd,NULL);
                    continue;
                }

                printf("recv(eventfd=%d,size=%ld):%s\n",ret[ii].data.fd,isize,buffer);

                write(ret[ii].data.fd,buffer,strlen(buffer));
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
