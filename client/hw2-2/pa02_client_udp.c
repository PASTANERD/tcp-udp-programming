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
#define SERVER_IP_ADDR "10.1.0.1" // 10.1.0.1 (203.252.119.34)

#define MESSAGE_END "THIS_MESSAGE_IS_END"
#define REQUEST_FILE "FILE_TRANS_REQUEST"

void error_handling(char *message);
void file_name_message(char *message, char *file_name); // 버퍼를 초기화하는 함수 
void sigAlarm(int);

int main(int argc, char *argv[]){
    FILE *file_stream;
    int file_size;

    int sock;
    char message[BUFSIZE];
    char rcvBuf[BUFSIZE];
    int str_len, addr_size;
    struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;
    
    int status;
    struct sigaction act;

    if(argc!=3){
	printf("Usage : %s <port> <file_name>\n", argv[0]);
	exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");
    
    // Setup for checking timeout
    act.sa_handler = sigAlarm;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    status = sigaction(SIGALRM, &act, 0);	
    
    if((file_stream = fopen(argv[2], "rb")) != NULL){

	// [STATE] Request, sending request message to server and check echo
	file_name_message(message, REQUEST_FILE);
	do{
	send(sock, message, BUFSIZE, 0);
	alarm(2);
	if((str_len = recv(sock, rcvBuf, BUFSIZE, 0)) < 0){
	    if(errno == EINTR){ // when timeout occurs
		errno = 0;
		fprintf(stderr, "REQUEST : socket timeout. \n");
		continue;
	    }
	    else error_handling("recv() error.");
	}
	else{
	    alarm(0);
	    //fprintf(stdout, "Request connected : %s \n", message);	
	}
	}while(strncmp(rcvBuf, REQUEST_FILE, sizeof(REQUEST_FILE)));

	// [STATE] Transmit file name, sending file name, end check echo 
	file_name_message(message,argv[2]);
    	fprintf(stdout, "file name : %s\n", message);
	do{
	send(sock, message, BUFSIZE, 0);
	alarm(2);
	if((str_len = recv(sock, rcvBuf, BUFSIZE, 0)) < 0){
	    if(errno == EINTR){
		errno = 0;
		fprintf(stderr, "FILE NAME : socket timeout. \n");
		continue;
	    }
	    else error_handling("recv() error");
	}
	else{
	    alarm(0);
	    fprintf(stdout, "File name transmitted : %s \n", message);
	}
	}while(strncmp(rcvBuf, argv[2], sizeof(argv[2])));
	
    }
    else error_handling("file open failed. check the file exists");

	
    // [STATE] Send all sources in file
    while((str_len = fread(message, 1, BUFSIZE, file_stream)) > 0){
	send(sock, message, str_len, 0);
    }
    file_name_message(message, MESSAGE_END);
    send(sock, message, BUFSIZE, 0);
    
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

void sigAlarm(int signo){
    //fprintf(stderr, "alarm\n");
    return;
}
