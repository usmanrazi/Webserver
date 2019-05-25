#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

int commandOptions(int argc, char ** argv, char ** portno, int status);
int Server(int argc, char ** argv, char ** portno, int status);
void ClientRequests();
int PrintInputOptions();
char* Response(char * message);
