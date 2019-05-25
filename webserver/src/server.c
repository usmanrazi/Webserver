#include "server.h"

int main(int argc, char **argv){
	char * portno = NULL;
	int status = 0;
	int ValidCommand = commandOptions(argc, argv, &portno, status);
	int ServerSuccessful = Server(argc, argv, &portno, status);
	if(ValidCommand==0 && ServerSuccessful== 0 ){
		printf("Connection Established.\n");
		ClientRequests();
	}
	return(0);
}
