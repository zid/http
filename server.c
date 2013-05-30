#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "socket.h"
#include "server.h"
#include "client.h"

struct server {
	int sock;
	int pollfd;
};

/* Register a file descriptor with epoll */
static int server_epoll_add_fd(Server *s, int fd)
{
	struct epoll_event ev = {0};
	int r;

	ev.events = EPOLLIN;
	ev.data.fd = fd;
	r = epoll_ctl(s->pollfd, EPOLL_CTL_ADD, fd, &ev);
	if(r == -1)
	{
		fprintf(stderr, "Fatal: Couldn't add fd to epoll.\n");
		server_destroy(s);
		return -1;
	}
	/* TODO: Do something with ev? */
	return 0;
}

static int server_epoll_remove_fd(Server *s, int fd)
{
	int r;

	if(fd == s->pollfd)
	{
		fprintf(stderr, "Fatal: You just removed the server fd from epoll you retard.\n");
		return -1;
	}

	r = epoll_ctl(s->pollfd, EPOLL_CTL_DEL, fd, NULL);
	if(r == -1)
	{
		fprintf(stderr, "Fatal: Couldn't remove fd from epoll.\n");
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

	if(!client_new(fd))
		return;	/* Too many clients */

	server_epoll_add_fd(s, fd);
}

static void server_read(Server *s, int fd)
{
	char buf[4096];
	int n;

	n = socket_read(fd, buf, 4096);

	if(n <= 0)
		goto kill;

	if(client_read_data(fd, buf, n))
		goto kill;

	return;

	kill:
	client_destroy(fd);
	server_epoll_remove_fd(s, fd);
	socket_close(fd);
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
		int fd = ev[n].data.fd;

		if(fd == s->sock)
		{
			server_accept(s);
		} else {
			server_read(s, fd);
		}

	}

	return 0;
}

void server_destroy(Server *s)
{
	free(s);
}



