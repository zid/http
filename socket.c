#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include "socket.h"

int socket_accept(int sock)
{
	struct sockaddr_storage addr;
	socklen_t addr_size;
	int fd;

	fd = accept(sock, (struct sockaddr *)&addr, &addr_size);

	return fd;
}

int socket_listen_create(const char *port)
{
	struct addrinfo hints, *info, *p;
	int r, sock;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family	  = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags	  = AI_PASSIVE;

	r = getaddrinfo(NULL, port, &hints, &info);
	if(r)
	{
		fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(r));
		return -1;
	}

	for(p = info; p; p = p->ai_next)
	{
		int yes;

		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sock == -1)
		{
			fprintf(stderr, "warning: Unable to create socket.\n");
			continue;
		}

		r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if(r == -1)
		{
			fprintf(stderr, "warning: Unable to set SO_REUSEADDR on socket.\n");
			continue;
		}

		r = bind(sock, p->ai_addr, p->ai_addrlen);
		if(r == -1)
		{
			fprintf(stderr, "warning: Unable to bind on socket.\n");
			continue;
		}

		break;
	}

	if(!p)
	{
		fprintf(stderr, "Fatal: Unable to create socket.\n");
		return -1;
	}

	freeaddrinfo(info);

	r = listen(sock, 10);
	if(r == -1)
	{
		fprintf(stderr, "Fatal: Unable to listen on socket.\n");
		return -1;
	}

	return sock;
}
