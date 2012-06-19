#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "socket.h"
#include "server.h"

struct server {
	int sock;
	int pollfd;
};

/* Register a file descriptor with epoll */
static int server_epoll_add_fd(Server *s, int fd)
{
	struct epoll_event ev;
	int r;

	ev.events = EPOLLIN;
	r = epoll_ctl(s->pollfd, EPOLL_CTL_ADD, fd, &ev);
	if(r == -1)
	{
		fprintf(stderr, "Fatal: Couldn't add socket to epoll.\n");
		server_destroy(s);
		return -1;
	}

	return 0;
}

static void server_accept(Server *s)
{
	int fd;

	fd = socket_accept(s->sock);
	if(fd == -1)
		return;

	server_epoll_add_fd(s, fd);
}

Server *server_create(const char *port)
{
	Server *s;

	s = malloc(sizeof(Server));

	s->sock = socket_listen_create(port);
	if(s->sock == -1)
		return NULL;

	s->pollfd = epoll_create(1024);
	if(s->pollfd == -1)
	{
		fprintf(stderr, "Fatal: Couldn't create epoll fd.\n");
		server_destroy(s);
		return NULL;
	}

	if(server_epoll_add_fd(s, s->sock) == -1)
	{
		fprintf(stderr, "Fatal: Couldn't add fd to epoll.\n");
		server_destroy(s);
		return NULL;
	}

	return s;
}

int server_do(Server *s)
{
	static struct epoll_event *ev;
	int n, nfds;

	if(!ev)
		ev = calloc(10, sizeof(struct epoll_event));

	nfds = epoll_wait(s->pollfd, ev, 10, -1);

	for(n = 0; n < nfds; n++)
	{
		if(ev[n].data.fd == s->sock)
		{
			server_accept(s);
		} else {
			/* server_read_fd() */
		}

	}
	return 0;
}

void server_destroy(Server *s)
{
	free(s);
}



