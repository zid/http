#ifndef CLIENT_H
#define CLIENT_H

void client_new(int);
void client_destroy(int);
int client_read_data(int, char *, int);
#endif
