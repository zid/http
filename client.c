#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "socket.h"
#include "client.h"

#define MAX_LINE 8192

struct client {
	char *line;
	int filled;
	int active;
};

static struct client c[256];

void client_destroy(int fd)
{
	c[fd].active = 0;

	free(c[fd].line);
}

static int find_crlf(const char *buf, int len)
{
	const char crlf[2] = {'\r', '\n'};
	char *found;

	found = memmem(buf, len, crlf, 2);
	if(!found)
		return -1;

	return found-buf;
}

/* A full line has been sent by the browser ready for us to do something 
 * with.  */
static int client_parse(int fd)
{
	const char msg[] = "Arsebandits";

//	printf("Client %d said '%s'\n", fd, c[fd].line);	

	if(c[fd].line[0] == '\0')
	{
//		printf("Telling client to gtfo\n");
		write(fd, msg, sizeof(msg)-1);
		return 1;
	}

	return 0;
}

int client_read_data(int fd, char *buf, int n)
{
	int bytes_to_copy;
	int pos, overhang;

	if(c[fd].filled + n > MAX_LINE)
		bytes_to_copy = MAX_LINE - c[fd].filled;
	else
		bytes_to_copy = n;

	memcpy(&c[fd].line[c[fd].filled], buf, bytes_to_copy);

	c[fd].filled += bytes_to_copy;

	while(1)
	{
		int r;

		pos = find_crlf(c[fd].line, c[fd].filled);

		if(pos == -1)
		{
		if(c[fd].filled == MAX_LINE)
			{
				client_destroy(fd);
				return 1;
			}

			/* Buffer isn't full, but we haven't found crlf either */
			return 0;
		}

		/* We have found a crlf! */
		/* Replace the \r\n with a terminator */
		c[fd].line[pos] = '\0';
		r = client_parse(fd);
		if(r)
			return 1;

		/* If we had any overhang past the line
		 * move it to the start of the buffer here.
		 */
		overhang = c[fd].filled - (pos + 2);
		if(overhang == 0)
			break;

		memmove(c[fd].line, &c[fd].line[pos+2], overhang);
		c[fd].filled = overhang;

		/* Go and search for clrf again, to see if we read
		 * data with multiple lines in it.
		 */
	}

	return 0;
}

int client_new(int fd)
{
	if(fd >= 256)
	{
		fprintf(stderr, "Warn: Too many connections.\n");
		socket_close(fd);
		return 0;
	}

	if(c[fd].active == 1)
	{
		fprintf(stderr, "Fatal: Duplicate file descriptor.\n");
		exit(EXIT_FAILURE);
	}

	c[fd].active  = 1;
	c[fd].filled  = 0;
	c[fd].line    = malloc(MAX_LINE);
	c[fd].line[0] = '\0';

	return 1;
}

