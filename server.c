#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#define BUF_SIZE 100
#define PORT 8080


int main(int argc, char *argv[])
{
	int status, socketfd, new_socketfd, fd_count, fd_size;
	struct addrinfo hints, *res, *p;
	struct sockaddr_in client_addr, *ipv4;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
	socklen_t addr_size;
	char ipstr[INET_ADDRSTRLEN];
	char buf[BUF_SIZE];
	void *addr;
	ssize_t msg_size;


	if (argc != 2) {
	    fprintf(stderr,"usage: broadcast start\n");
	    return 1;
	}
    

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    

	if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

	for(p = res;p != NULL; p = p->ai_next) {



		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(socketfd == -1){
			fprintf(stderr, "error invoking the file descriptor: %s\n", strerror(errno));
			continue;
		}

		if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int)) == -1) {
			fprintf(stderr, "error reusing the socket: %s\n", strerror(errno));
			exit(1);
		}


		if(bind(socketfd, p->ai_addr, p->ai_addrlen) == -1){
			close(socketfd);
			fprintf(stderr, "error binding: %s\n", strerror(errno));
			continue;
		}

		break;
	}

    freeaddrinfo(res); // free the linked list

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	
	if(listen(socketfd, 5) == -1){
		fprintf(stderr, "error listening: %s\n", strerror(errno));
		exit(1);
	}

	pfds[0].fd = socketfd;
	pfds[0].events = POLLIN;

	fd_count = 1;

	printf("server: waiting for connections...\n");

	while(1){

		int poll_count = poll(pfds, fd_count, -1);
		if(poll_count == -1){
			fprintf(stderr, "error polling: %s\n", strerror(errno));
			exit(1);
		}

		
		for(int i = 0; i < fd_count; i++){
			if(pfds[i].revents & (POLLIN | POLLHUP)){
				if(pfds[i].fd == socketfd){                // new connection
					addr_size = sizeof client_addr;
					new_socketfd = accept(socketfd, (struct sockaddr *)&client_addr, &addr_size);
					if(new_socketfd == -1){
						fprintf(stderr, "error accepting: %s\n", strerror(errno));
						continue;
					}
					if(fd_count >= fd_size){
						fd_size *= 2;
						realloc(pfds, sizeof *pfds * fd_size);
					}
					pfds[fd_count].fd = new_socketfd;
					pfds[fd_count].events = POLLIN;
					pfds[fd_count].revents = 0;
					fd_count++;
					addr = &(client_addr.sin_addr);
					inet_ntop(client_addr.sin_family, addr, ipstr, sizeof ipstr);
					printf("server: got connection from %s\n", ipstr);
				} else{
					msg_size = recv(pfds[i].fd, &buf, sizeof buf, 0);
				}

			}
		}
		


	}
	free(pfds);
	
	

	return 0;
}