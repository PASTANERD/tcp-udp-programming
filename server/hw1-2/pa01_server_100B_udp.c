#include <stdio.h>	
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 100

void error_handling(char *message);

int main(int argc, char* argv[]){
	int serv_sock;
	char message[BUFSIZE];
	int str_len;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;

	if(argc!=2){
		printf("Usage : %s <port> \n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1) error_handling("UDP socket generating error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");

	sleep(10);

	while(1){
		clnt_addr_size=sizeof(clnt_addr);
		str_len = recvfrom(serv_sock, message, BUFSIZE, 0, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
		if(!(strcmp(message, "q")-10)){
			printf("client process is terminated.\n");
			break;
		}
		write(1, message, str_len);
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
