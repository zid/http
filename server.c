#include <stdlib.h>
#include "socket.h"
#include "server.h"

struct server {
	int sock;
};

Server *server_create(const char *port)
{
	Server *s;

	s = malloc(sizeof(Server));

	s->sock = socket_listen_create(port);
	if(s->sock == -1)
		return NULL;

	return s;
}

int server_do(Server *s)
{
	/* epoll */
	(void)s;
	return 0;
}

void server_destroy(Server *s)
{
	free(s);
}



