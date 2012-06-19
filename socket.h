#ifndef SOCKET_H
#define SOCKET_H

int socket_listen_create(const char *);
int socket_accept(int);
int socket_read(int, char *, int);
void socket_close(int);
#endif
