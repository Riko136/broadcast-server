#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "8080"
#define BUF_SIZE 256

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status, socketfd;
	char ipstr[INET_ADDRSTRLEN], buf[BUF_SIZE], buf_recv[BUF_SIZE];
	struct sigaction sa;
	pid_t pid;


	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    return 1;
	}
    

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  
	hints.ai_socktype = SOCK_STREAM;
    

	if ((status = getaddrinfo(argv[1], PORT, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		struct sockaddr_in *ipv4;

	    socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(socketfd == -1){
			fprintf(stderr, "error invoking the file descriptor: %s\n", strerror(errno));
			continue;
		}

		
		if(connect(socketfd, p->ai_addr, p->ai_addrlen) == -1){
			fprintf(stderr, "error connecting: %s\n", strerror(errno));
			close(socketfd);
			continue;
		}

		ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr = &(ipv4->sin_addr);
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("client connecting to the server: %s", ipstr);
		break;
	}



	if(p == NULL){
		fprintf(stderr, "error connecting to the server: address unkown\n");
		exit(1);
	}
	freeaddrinfo(res); // free the linked list

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	
	pid = fork();
	switch(pid){
		case -1:
			fprintf(stderr, "fork error");
			exit(1);
		case 0:
			while(fgets(buf, BUF_SIZE, stdin) != NULL){
				if(send(socketfd, buf, sizeof buf, 0) == -1){
					fprintf(stderr, "error sending the message: %s\n", strerror(errno));
					exit(1);
				}
			}
			puts("fgets while loop terminated");
			exit(0);
		default:
			while(recv(socketfd, buf_recv, sizeof buf_recv, 0) > 0){
				puts(buf_recv);
			}
			fprintf(stderr, "error recieving the message: server either terminated or something went wrong\n");
			exit(1);
	}




}