#ifndef SERVER_H
#define SERVER_H

typedef struct server Server;

Server *server_create(const char *);
int server_do(struct server *);
void server_destroy(struct server *);

#endif
