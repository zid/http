#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "server.h"

int main(int argc, char *argv[])
{
	int c, option_index;
	char port[16] = "8000";
	Server *s;

	static struct option options[] = {
		{"port", required_argument, 0, 'p'},
		{NULL, 0, 0, 0}
	};

	while(1)
	{
		c = getopt_long(argc, argv, "p:", options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 'p':
				/* No need to implement safe strcpy */
				*port = 0;
				strncat(port, optarg, 15);
			break;
		}
	
	}

	s = server_create(port);
	if(!s)
		return EXIT_FAILURE;
	while(1)
	{
		int r;

		r = server_do(s);

		if(r == -1)
			return EXIT_FAILURE;

		if(r)
			break;
	}

	return EXIT_SUCCESS;
}
