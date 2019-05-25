#include "server.h"

#define DIE(str) perror(str);
#define BUFSIZE 4000

int commandOptions(int argc, char ** argv, char ** portno, int status){
	status =0;
	int c;
	int getopt();
	opterr = 0;
	c = getopt (argc, argv, "hp:s:");

	while( c != -1 ){

		if(c == 'h'){
			PrintInputOptions();
			printf("command h function.\n");

			break;
		}		
		else if(c == 'p'){

			PrintInputOptions();
			printf("command p function.\n");
			break;			
		}
		else if(c == 's'){
			PrintInputOptions();
			printf("command s function.\n");
			exit(3);			
			break;		
		}else if(c == '?'){
			if (optopt == 'p' || optopt == 's')
				printf ("ERROR: OPTION -%c NEEDS AN ARGUMENT\n", optopt);
			else
				printf("ERROR: OPTION/ARGUMENT NOT FOUND!\n");
			
			PrintInputOptions();
			printf("command ? function.\n");
			status =1;
			exit(3);			
			break;			
		}else{
			PrintInputOptions();
			status =1;
			exit(3);
			break;		
		}
	}
	return status;
}

int PrintInputOptions(){
	printf("\n");
	printf("-h Print help.\n");
	printf("-p Listen to port number.\n");
	printf("-s |fork| select request handling method.\n");
	printf("\n");
}

int sd;
int addrlen;
int Server(int argc, char ** argv, char ** portno, int status)
{
	struct sockaddr_in sin;
	status=0;
	int Value;
	int portnumber;
	char buf[BUFSIZE];
	
	portnumber = atoi(argv[2]);

	if(!portnumber)
		portnumber = 8080;

	/* get a file descriptor for an IPv4 socket using TCP */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if( sd ==  -1 )  {
		printf("Socket unable to find for connection\n");
		status = -1;
		DIE("socket");
	}else{
		int size =1;
		setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &size, sizeof(size));
	}

	/* zero out the sockaddr_in struct */
	memset(&sin, 0, sizeof(sin));
	/* setup the struct to inform the operating system that we would like
	* to bind the socket the the given port number on any network
	* interface using IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(portnumber);

	/* perform bind call */
	if(bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
		printf("Unable to bind\n");
		status = -1;		
		DIE("bind");
	}
	/* start listening for connections arriving on the bound socket */
	listen(sd, 10);
}

void ClientRequests(){
	
	struct sigaction sa;
	sa.sa_handler=SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=0;
	if(sigaction(SIGCHLD,&sa,0)==-1)
	{
		printf("%s\n","SIG ACTION");exit(1);
	}
	while(1)
	{
		struct sockaddr_in clientaddr;
		int sd_current;
		addrlen = sizeof(clientaddr);
		/* waiting for the incomming connections; */

		printf("Server is Ready for Connection.\n");
		sd_current = accept(sd, (struct sockaddr*) &clientaddr, &addrlen);

		if (sd_current == -1) 
		{
			printf("Server is not responding for connections.\n");
			DIE("accept");
		}
		else 
		{
			int pid = fork();
			if(pid<0)
				printf("%s\n","Error");
			if(pid==0)
			{
				close(sd);
				char Client_ipAddress[INET_ADDRSTRLEN];
				/* convert IP address of client partner to string */
				inet_ntop(AF_INET, &clientaddr.sin_addr, Client_ipAddress, sizeof(Client_ipAddress));

				char buf[BUFSIZE];
				/* receive at most sizeof(buf) many bytes and store them in the buffer */
				long OutputStatus = recv(sd_current, buf, sizeof(buf), 0);
				if (OutputStatus == -1) 
				{
					printf("Couldn't receive.\n");
					DIE("could-not-recveive");
				}
				else if(OutputStatus > BUFSIZE)
				{
					printf("Message is Bigger than allocated Buffer size.\n");
				}
				else
				{
					char *message=malloc(strlen(buf)) ;
					strcpy(message, buf);
					if(strlen(message) >0)
					{
							char*msg=Response(message);
							if(send(sd_current, msg, strlen(msg) , 0)>=0)
							{
								//printf("Message sent to the client.\n----\n%s\n", message);
							}
					}
					//shutdown(sd_current, SHUT_RDWR);
					//close(sd_current);
				}
				exit(0);
				//close(sd_current);	
			}
			close(sd_current);		
		}
	}
}

char* Response(char * message){
	int status = 200;
	int lineNum = 0;
	char *  line;
	char *  crnl;
	char* split_line[3];
	for (line = message;(crnl = strtok(line, "\r\n")) != NULL;line = crnl + 1) {
		++lineNum;
		if (lineNum == 1) {
			char * crnlSplit=strtok(crnl," ");
		    int curr_line = 0;
		    while(crnlSplit!=NULL){
				split_line[curr_line] = malloc(strlen(crnlSplit) * sizeof(char));
				strcpy(split_line[curr_line], crnlSplit);
				curr_line++;
				crnlSplit=strtok(NULL," ");
			}
		}
	}
	
	if(split_line == NULL){
		printf("Request Must have a Method and Path file\n");
		status = 400;
	}
		
	char*method;
	if(strcmp(split_line[0], "GET") || strcmp(split_line[0], "HEAD")){
		method=split_line[0];
	}
    
	char * full_path;
	char* directory = realpath("../www", NULL);
    
    if(directory == NULL){
		status = 500;
	}else{
		char * path = malloc(strlen(directory) + strlen(split_line[1]) + 1);
		strcat(path, directory);
		strcat(path, split_line[1]);
		char * real_path;
		real_path = realpath(path, NULL);

		if(strcmp(split_line[1], "/")==0){
			strcat(real_path, "/index.html");
			full_path = real_path;
			status=200;
		}else if(!real_path){
			printf(("Invalid Path\n"));
			status = 404;
		}else if(strncmp(directory, real_path, strlen(directory)) != 0){
			printf("Invalid file\n");
			status = 403;
		}else{
			full_path = real_path;
		}
	}
	
	char timebuffer[50];
	char last_modified[50]; 
    time_t t = time(NULL);
    struct tm timeStamp = *localtime(&t);
    strftime(timebuffer, sizeof(timebuffer), "%a, %d %b %Y %H:%S", &timeStamp);
    
    struct stat filestat;
    int size = 0;
    
    if(full_path!=NULL)
	{
		stat(full_path, &filestat);
		size = filestat.st_size;
		timeStamp=*localtime(&(filestat.st_mtime));
		strftime(last_modified, sizeof(last_modified), "%a, %d %b %Y %H:%S", &timeStamp);
	}
	else
	{
		strncpy(last_modified, timebuffer, 50);
	}
	
    char http_protocol[] = "HTTP/1.0";
	char request[] = "";
	char* html = NULL;
	long length = 0;
	char* full_msg = NULL;
	char* full_header_message=(char*)malloc(128); 
	char ch;
	
	if( (strcmp(method, "GET")==0) || (strcmp(method, "HEAD")==0) ){
		FILE *f = fopen (full_path, "rb");
        if (f)
        {
            fseek (f, 0, SEEK_END);
            length = ftell (f);
            rewind(f);
            html = malloc (length * sizeof(char) + 1);
            if (html)
            {
                fread (html, sizeof(char), length, f);
                html[length] = 0;
            }
            fclose (f);
        }
	}
	
	char * http_prt = (char *) malloc(25);
	char * http = (char *) malloc(15);
	strcpy(http_prt, split_line[2]);
	
	http = strtok (split_line[2],"/");
	while (http != NULL)
    {
		if(strcmp(http, "1.1") == 0)
        {
			status = 501;
		}
		http= strtok(NULL, " ");
    }
  
	char * status_Message;
	if(status == 200){
		status_Message="ok";
	}
	else if(status == 500){
		status_Message="internal server error";
	}
	else if(status == 400){
		status_Message="Bad Request";
	}
	else if(status == 403){
		status_Message="Forbidden";
	}
	else if(status == 404){
		status_Message="Not Found";
	}
	else{
		status_Message="Not Implemented";
	}
	
	if(status != 200)
    {
        int html_length = 500;
        char* head_msg = malloc(sizeof(char) * html_length);
        memset(head_msg, 0, sizeof(char) * html_length);
        
        sprintf(head_msg, 
				"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n"
                "<html>\n"
                "<head>\n"
                "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
                "<title>%d %s</title>\n"
                "</head>\n"
                "<body>\n"
                "<h1>ERROR: %d %s</h1>\n"
                "</body>\n"
                "</html>",
                status, status_Message, status, status_Message);
        
        length = strlen(head_msg);
        html = head_msg;
    }
    
    sprintf(full_header_message, "%s %i %s\nContent-Length: %ld\r\nContent-Type: text/html\r\nDate: %s\r\nLast-Modified: %s\r\nServer: Server\r\n", http_prt, status, status_Message, length, timebuffer, last_modified);
	full_msg = malloc(strlen(full_header_message)+length+2);
    strcpy(full_msg, full_header_message);
	
    if (html)
    {
        strcpy(full_msg+strlen(full_msg), "\n");
        strcpy(full_msg+strlen(full_msg), html);
    }
	return full_msg;
	free(html);
    free(full_path);
}




