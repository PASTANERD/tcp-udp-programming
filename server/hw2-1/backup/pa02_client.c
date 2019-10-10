#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 1024
#define SERVER_IP_ADDR "10.1.0.2"
int FILE_NAME_FLAG = 0;
void error_handling(char *message);
void file_name_message(char *message, char *file_name); // 버퍼를 초기화하는 함수 

int main(int argc, char *argv[]){
    FILE *file_stream;

    int sock;
    char message[BUFSIZE];
    int str_len;
    struct sockaddr_in serv_addr;

    if(argc!=3){
	printf("Usage : %s <port> <file_name>\n", argv[0]);
	exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");
    
    // 전송하고자하는 파일의 이름을 Argument로 받으면 해당 파일 스트림을 열고, 파일 이름을 서버로 전송한다.
    file_stream = fopen(argv[2], "rb");
    file_name_message(message,argv[2]);
    send(sock, message, strlen(argv[2]), 0);
   
    // 파일 내용을 바로 전송한다. 
    while((str_len = fread(message, 1, BUFSIZE, file_stream)) > 0){
	fputs(message, stdout);
	send(sock, message, str_len, 0);
    }

    // 전송이 완료되면 스트림과 디스크립터를 해제한다.    
    fclose(file_stream);
    close(sock);
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

