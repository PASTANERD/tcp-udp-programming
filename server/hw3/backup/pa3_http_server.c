#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 100
//#define ASIGNED_PORT 80

void error_handling(char *message);

int main(int argc, char *argv[]){
	int serv_sock;
	int clnt_sock;
	char message[BUFSIZE];
	int str_len;
	const char * response_packet = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nHello";

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	if(argc != 2){
		printf("Usage : %s <port>", argv[0]);
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
	
	clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
	if(clnt_sock == -1) error_handling("accept() error");
	
	/*
	str_len = recv(clnt_sock, message, BUFSIZE, 0);
	if(str_len > 0) printf("message : %s \n", message);
	send(clnt_sock, response_packet, strlen(response_packet), 0);
	*/

	while(1){
//		clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
//		if(clnt_sock == -1) error_handling("accept() error");
//		else printf("http connect recepted.\n");
		printf("test\n");
		str_len = recv(clnt_sock, message, BUFSIZE, 0);
		if(str_len > 0) printf("message : %s \n", message);
		send(clnt_sock, response_packet, strlen(response_packet), 0);		
		
	}

	close(clnt_sock);
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
