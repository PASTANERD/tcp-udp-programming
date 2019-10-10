#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 30
#define SERVER_IP_ADDR "203.252.119.34"
#define PRE_MSG_01 "0123456789"
#define PRE_MSG_02 "ABCDEFGHIJ"
#define PRE_MSG_03 "KLMNOPQRST"

void error_handling(char *message);

int main(int argc, char *argv[]){
    int sock;
    char message[BUFSIZE];
    int str_len;

    struct sockaddr_in serv_addr;

    if(argc!=2){
	printf("Usage : %s <port> \n", argv[0]);
	exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock == -1) error_handling("UDP socket generating error.");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(SERVER_IP_ADDR);
    serv_addr.sin_port=htons(atoi(argv[1]));

    sendto(sock, PRE_MSG_01, strlen(PRE_MSG_01), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    sendto(sock, PRE_MSG_02, strlen(PRE_MSG_02), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    sendto(sock, PRE_MSG_03, strlen(PRE_MSG_03), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));


	while(1){
		fputs("To send message (q to quit) : ", stdout);
		fgets(message, BUFSIZE, stdin);

		sendto(sock, message, strlen(message), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

		if(!strcmp(message, "q\n")) break;
	}

     close(sock);
     return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
