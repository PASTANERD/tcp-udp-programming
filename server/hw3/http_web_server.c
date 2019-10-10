#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>

#define LOG 10
#define ERROR 20

#define CODE200 200
#define CODE404 404

#define PHRASE200 "OK"
#define PHRASE404 "FILE NOT FOUND"

char documentRoot[] = "/home/s21400646/hw3/data";
struct extensions{
  char *ext;
  char *filetype;
};

void do_web(int);
void web_log(int, char[], char[], int);
void content_type(char*, char*, char*, struct extensions*);
void make_sample(int);

int log_fd;

int main(int argc, char *argv[]){
  struct sockaddr_in s_addr, c_addr;
  int s_sock, c_sock;
  int len, len_out;
  unsigned short port;
  int status;
  struct rlimit resourceLimit;
  int i;

  int pid;

  if(argc != 2){
    printf("Usage : %s, <port_number>", argv[0]);
    exit(1);
  }
  
    if(fork() != 0) return 0;

    (void)signal (SIGCHLD, SIG_IGN);
    (void)signal (SIGHUP, SIG_IGN);
    resourceLimit.rlim_max = 0;
    status = getrlimit(RLIMIT_NOFILE, &resourceLimit);
    for(i = 0; i < resourceLimit.rlim_max ; i++){
        close(i);
    }

  web_log(LOG, "STATUS", "web server start", getpid());

  if((s_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    web_log(ERROR, "SYSCALL", "web server sockect for listening (s_sock) open error.", s_sock);
  }

  port = atoi(argv[1]);
  if(port > 60000) web_log(ERROR, "ERROR", "invalid port number", port);

  memset(&s_addr, 0, sizeof(s_addr));
  s_addr.sin_family = AF_INET;
  s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  s_addr.sin_port = htons(port);

  if(bind(s_sock, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0) web_log(ERROR, "ERROR", "server cannot bind", 0);

  listen(s_sock, 10);

  while(1){
    len = sizeof(c_addr);
    if((c_sock = accept(s_sock, (struct sockaddr*) &c_addr, &len)) < 0) web_log(ERROR, "ERROR", "server accpet error", 0);

    if((pid = fork()) < 0){
      web_log(ERROR, "ERROR", "server fork error", 0);
    } else if (pid == 0){
      close(s_sock);  
      do_web(c_sock);
    }
    else {
      close(c_sock);
    }
  }
}

void do_web(int c_sock){
  char sndBuf[BUFSIZ+1], rcvBuf[BUFSIZ+1];
  char uri[100], c_type[20];

  int len_out;
  int n, i;
  char *p;
  char method[10], f_name[20];
  char phrase[20] = "OK";

  int code = 200;
  int fd;

  char file_name[20];
  char ext[20];

  struct stat sbuf;
struct extensions extensions[] = {
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"zip", "image/zip"},
    {"gz", "image/gz"},
    {"tar", "image/tar"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {0,0} };
  
  memset(rcvBuf, 0, sizeof(rcvBuf));
  if((n = read(c_sock, rcvBuf, BUFSIZ)) <= 0) web_log(ERROR, "ERROR", "can not receive data from web browser", n);
  
  web_log(LOG, "REQUEST", rcvBuf, n);
  
  p = strtok(rcvBuf, " ");
  if(!strcmp(p, "GET") || !strcmp(p, "get")){
    content_type(p, uri, c_type, extensions);
      if((fd = open(uri, O_RDONLY)) < 0){
	code = CODE404;
	strcpy(phrase, PHRASE404);
    }
  }
  else if(!strcmp(p, "POST") || !strcmp(p, "post")){
    content_type(p, uri, c_type, extensions);
    fd = creat(uri, 0644);
    make_sample(fd);
    fd = open(uri, O_RDONLY);
  } 
  else web_log(ERROR, "ERROR", "Only get and post method can be supported", 0); 

  sprintf(sndBuf, "HTTP/1.0 %d %s\r\n", code, phrase);
  n = write(c_sock, sndBuf, strlen(sndBuf));
  web_log(LOG, "RESPONSE", sndBuf, getpid());

  sprintf(sndBuf, "content-type: %s\r\n\r\n", c_type);
  n = write(c_sock, sndBuf, strlen(sndBuf));

  web_log(LOG, "RESPONSE", sndBuf, getpid());

  if(fd >= 0){
    while((n = read(fd, rcvBuf, BUFSIZ)) > 0){
      write(c_sock, rcvBuf, n);
    }
  }

  close(c_sock);
  exit(1);
}

void web_log(int type, char s1[], char s2[], int n){
  int log_fd;
  char buffer[BUFSIZ];

  if(type == LOG){
    sprintf(buffer, "[STATUS] %s %s %d\n", s1, s2, n);
  }else if(type == ERROR){
    sprintf(buffer, "[ERROR] %s %s %d\n", s1, s2, n);
  }

  if((log_fd = open("web.log", O_CREAT|O_WRONLY|O_APPEND, 0644)) >= 0){
    write(log_fd, buffer, strlen(buffer));
    close(log_fd);
  }

  if(type == ERROR) exit(1);
}

void content_type(char *p, char *uri, char *c_type, struct extensions *extensions){
  int i, len;

  p = strtok(NULL, " ");
  if(!strcmp(p, "/")) sprintf(uri, "%s/index.html", documentRoot);
  else sprintf(uri, "%s%s", documentRoot, p);

  strcpy(c_type, "text/plain");
  for(i=0; extensions[i].ext != 0; i++){
    len = strlen(extensions[i].ext);
    if(!strncmp(uri+strlen(uri)-len, extensions[i].ext, len)){
      strcpy(c_type, extensions[i].filetype);
      break;
    }
  }

} 

void make_sample(int fd){
  int i;
  char html_header[] = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>Sample</title>\n</head>\n";
  char html_body1[] = "<body>\n\t<h2>";
  char html_body2[] = "</h2>\n</body>\n</html>";

  char buffer[BUFSIZ];
  char *dump;
  char *name;
  char *snumber;

  for(i = 0 ; i < 13 ; i++) dump = strtok(NULL, "\r\n");
  name = strtok(NULL, "&");
  snumber = strtok(NULL, "&");
  sprintf(buffer, "%s%s%s %s%s", html_header, html_body1, name, snumber, html_body2);
  write(fd, buffer, strlen(buffer));
  close(fd);
}
