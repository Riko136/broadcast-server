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

#define BUF_SIZE 256
#define PORT "8080"
void handle_client_data(struct pollfd *pfds, int fd_count, int *i){
ssize_t recv_size, send_size;
char buf[BUF_SIZE];

recv_size = recv(pfds[*i].fd, &buf, sizeof buf, 0);
if(recv_size <= 0){
	if(recv_size == 0){
		printf("pollserver: socket %d hung up\n", pfds[*i].fd);
	} else{
		fprintf(stderr, "error recieving the message: %s", strerror(errno));
	}

	close(pfds[*i].fd);
	for(int j = *i; j < fd_count; j++){
		pfds[j] = pfds[j+1];
	}
	(*i)--;

} else{
	for (int j = 1; j < fd_count; j++)
	{
		if(pfds[j].fd != pfds[*i].fd)
		{
			send_size = send(pfds[j].fd, &buf, sizeof buf, 0);
			if(send_size == -1){
				fprintf(stderr, "error sending the message: %s", strerror(errno));
			}else if (send_size < recv_size)
			{
				/* send the rest of the message */
			}
		}
	}
}
}

void handle_new_connection(int socketfd, struct pollfd **pfds, int *fd_count, int *fd_size){
	int new_socketfd;
	socklen_t addr_size;
	struct sockaddr_in client_addr;
	void *addr;
	char ipstr[INET_ADDRSTRLEN];

	addr_size = sizeof client_addr;
	new_socketfd = accept(socketfd, (struct sockaddr *)&client_addr, &addr_size);
	if(new_socketfd == -1){
		fprintf(stderr, "error accepting: %s\n", strerror(errno));
	}else{
		if((*fd_count) >= (*fd_size)){
			*fd_size *= 2;
			*pfds = realloc(*pfds, sizeof (**pfds) * (*fd_size));
		}
		(*pfds)[*fd_count].fd = new_socketfd;
		(*pfds)[*fd_count].events = POLLIN;
		(*pfds)[*fd_count].revents = 0;
		(*fd_count)++;
		addr = &(client_addr.sin_addr);
		inet_ntop(client_addr.sin_family, addr, ipstr, sizeof ipstr);
		printf("server: got connection from %s\n", ipstr);
	}

}
int main(int argc, char *argv[])
{
	int status, socketfd, fd_count, fd_size;
	int yes = 1;
	struct addrinfo hints, *res, *p;
	struct sockaddr_in *ipv4;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);



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

		if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
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
				if(pfds[i].fd == socketfd){                
					handle_new_connection(socketfd, &pfds, &fd_count, &fd_size);
				} else{	
					handle_client_data(pfds, fd_count, &i);
				}

			}
		}
	}
	free(pfds);
}