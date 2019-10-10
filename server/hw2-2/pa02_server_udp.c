#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#define BUFSIZE 50

#define MESSAGE_END "THIS_MESSAGE_IS_END"
#define REQUEST_FILE "FILE_TRANS_REQUEST"

#define STATE_STEADY 100
#define STATE_NAME_REQUEST 200
#define STATE_FILE_DOWNLOAD 300

void error_handling(char *message);
void sigAlarm(int);
void file_name_message(char *message, char *file_name);

int main(int argc, char *argv[]){
	FILE* file_stream;
	char* file_name;

	int status = STATE_STEADY;

	int serv_sock;
	char message[BUFSIZE];
	char echom[BUFSIZE];
	int str_len;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	int wait_state;
	struct sigaction act;

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1) error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");

	// Setup for checking timeout	
	act.sa_handler = sigAlarm;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	wait_state = sigaction(SIGALRM, &act, 0);
	

	while(1){
		clnt_addr_size = sizeof(clnt_addr);
		str_len = recvfrom(serv_sock, message, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
		
		// [STATE] Steady, to react when request message arrived
		if(!strncmp(message, REQUEST_FILE, strlen(REQUEST_FILE))) {
			if(errno == EINTR){ // if timeout, turn off alarm and steady
				errno = 0;
				fprintf(stderr, "socket timeout. : STATE_STEADY\n");
				alarm(0);
				continue;
			}
			sendto(serv_sock, message, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr));
			status = STATE_NAME_REQUEST;
			alarm(60); // let alarm check timeout for 60s
			continue;
		}

		// [STATE] Get file name, wait for file name
		if(status == STATE_NAME_REQUEST){
			if(errno == EINTR){ // if timeout, echo again (Request)
				errno = 0;
				fprintf(stderr, "socket timeout. : STATE_NAME_REQUEST\n");
				alarm(5);
				sendto(serv_sock, REQUEST_FILE, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr));
				continue;
			}
			file_name=malloc(sizeof(char)*strlen(message));
			strcpy(file_name, message);
			printf("file name = %s\n", file_name);
			file_stream = fopen(file_name, "wb");
			errno = 0 ;

			sendto(serv_sock, message, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr));
			status = STATE_FILE_DOWNLOAD;
			alarm(30);
			continue;
		}

		// [STATE] Download file, get all the data of file. When timeout, finish download
		if(status == STATE_FILE_DOWNLOAD){ 
			if(errno == EINTR){
				errno = 0;
				alarm(0);
				fprintf(stderr, "socket timeout. : STATE_FILE_DOWNLOAD \n", message);
				status = STATE_STEADY;
				fclose(file_stream);
				fputs("\nfile download completed.\n", stdout);
			}
			if(!strncmp(message, MESSAGE_END, strlen(MESSAGE_END))) {
				errno = 0;
				alarm(0);
				fprintf(stderr, "\nEnd of File : %s\n", message);
				status = STATE_STEADY;
				fclose(file_stream);
				fputs("file download completed.\n", stdout);
			}
			else if(!strncmp(message, file_name, strlen(file_name)))
				sendto(serv_sock, file_name, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, sizeof(clnt_addr));
			else{
				//alarm(0);
				fputs(".", stdout);
				fwrite(message, 1, str_len, file_stream);
			}
		}
	}
	if(file_stream != NULL) fclose(file_stream);
	free(file_name);
	close(serv_sock);
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void file_name_message(char* message, char *file_name){
	memset(message, 0, sizeof(char)*BUFSIZE);
	strcpy(message, file_name);
}

void sigAlarm(int signo){
    fprintf(stderr, "alarm\n");
    return;
}
