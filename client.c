#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status, socketfd;
	char ipstr[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    return 1;
	}
    

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  
	hints.ai_socktype = SOCK_STREAM;
    

	if ((status = getaddrinfo(argv[1], "8080", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
        struct sockaddr_in *ipv4;
	
		ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr = &(ipv4->sin_addr);

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		if(ipstr != argv[1]){
			fprintf(stderr, "invalid ip adress: %s\n", ipstr);
			return 2;
		}
	}

    socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(socketfd < 0){
		fprintf(stderr, "error invoking the file descriptor: %s\n", strerror(errno));
		return 2;
	}

	status = connect(socketfd, res->ai_addr, res->ai_addrlen);
	if(status < 0){
		fprintf(stderr, "error connecting: %s\n", strerror(errno));
		return 2;
	}

	freeaddrinfo(res); // free the linked list

	return 0;
}