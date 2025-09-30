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


int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	struct sockaddr_in their_addr, *ipv4;
	struct pollfd *pfds = malloc(sizeof *pfds * 5);
	socklen_t addr_size;
	int status, socketfd, new_socketfd, fd_count;
	char ipstr[INET_ADDRSTRLEN];
	void *addr;


	if (argc != 2) {
	    fprintf(stderr,"usage: broadcast start\n");
	    return 1;
	}
    

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    

	if ((status = getaddrinfo(NULL, "8080", &hints, &res)) != 0) {
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
		
		for(int i = 0; i <= fd_count; i++ ){
			if(pfds[i].revents & POLLIN){
				
			}
		}
		
		

		







		addr_size = sizeof their_addr;
		new_socketfd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);

		if(new_socketfd == -1){
			fprintf(stderr, "error accepting: %s\n", strerror(errno));
			continue;
		}

		
		addr = &(their_addr.sin_addr);
		inet_ntop(their_addr.sin_family, addr, ipstr, sizeof ipstr);
		printf("server: got connection from %s\n", ipstr);


	}
	
	

	return 0;
}