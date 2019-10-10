#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 4096

void error_handling(char *message);

int main(int argc, char *argv[]){
	FILE* file_stream;
	char* file_name;
	int serv_sock;
	int clnt_sock;
	char message[BUFSIZE];
	int str_len;
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1) error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");
	
	if(listen(serv_sock, 5) == -1) error_handling("listen() error");
	
	clnt_addr_size = sizeof(clnt_addr);

	while(1){

		clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
		if(clnt_sock == -1) error_handling("accept() error");
	
		str_len = recv(clnt_sock, message, BUFSIZE, 0);
		file_name=malloc(sizeof(char)*strlen(message));
		strcpy(file_name, message);
		printf("file_name = %s\n", file_name);
		//memset(message, 0, sizeof(message));
		if((file_stream = fopen(file_name, "wb")) != NULL){	
		
			while((str_len=recv(clnt_sock, message, BUFSIZE, 0)) > 0){
				fwrite(message, 1, str_len, file_stream);
			}

		fputs("file download completed.\n", stdout);
		fclose(file_stream);
		close(clnt_sock);

		}
		else{
			error_handling("file open failed");
		}
	}
	free(file_name);
	close(serv_sock);
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

