#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX 80 
#define PORT 5555 
#define LISTENQ 10

void str_echo(int connfd) {
	char buff[MAX];
	int n;
	while (1) {
		bzero(buff, MAX);
		n = read(connfd, buff, sizeof(buff));
		if (n <= 0) {
            printf("Client disconnected or read error.\n");
            break;
        }
		printf("From Client: %s", buff);

		write(connfd, buff, n);
	}
}


int main() 
{ 
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket creation failed..\n");
		exit(1);
	} else {
		printf ("socket successfully created..\n");
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("socket bind failed...\n");
		exit(1);
	} else {
		printf("socket successfully binded...\n");
	}

	if (listen(listenfd, LISTENQ) < 0) { 
		perror("server listen failed...\n"); 
		exit(1); 
	} else {
		printf("server listening...\n");
	}

	while(1) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
		if (connfd < 0) {
			perror("server accept failed...\n");
			exit(1);
		} else {
			printf("server accepted client from %s : %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
		}
		
		if ((childpid = fork()) == 0) {
			close(listenfd);
			str_echo(connfd);
			exit(0);
		}
		close(connfd); 
	}
}
