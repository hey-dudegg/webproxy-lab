#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/*********************************************************
[client]---(request)--->[proxy]---(request)--->[server]
1. 명시된 포트로부터 들어오는 요청을 대기
2. 연결이 맺어지면 요청을 읽고 파싱(해석 / 분할)
3. 올바른 HTTP 요청인지 검증
4. 올바르다면 서버와 연결 후 요청 전달

// 아이디어들
-> uri_parse 함수 수정 -> host, port, path 분리하기
=> http://localhost:port/home 에서 host는 localhost, port는 :과/사이, path는 /home
-> clientfd = open_clinetfd(host, port)
-> Rio_readinit(&rio, clientfd)
-> Rio_weitten()

// echoserver와 echoclient를 참고해서 프록시를 구현하려 했으나 실패.
// 새로운 함수를 작성하는 것 외에 아이디어가 떠오르지 않음.
// uri_parse 함수 작성 중(test.c로 연습) 중단
**********************************************************/

/* You won't lose style points for including this long line in your code */
// static const char *user_agent_hdr =
//     "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
//     "Firefox/10.0.3\r\n";

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
// void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main() {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  
    if (argc != 2) { 
    /* fprintf(). 스트림에 형식화된 데이터 쓰기. 일련의문자와 값을 형식화하여 출력 stream에 쓴다. */
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    // 반복적으로 연결 요청을 접수합니다.
    c_connfd_p = Accept(listenfd, (SA *)&clientaddr,&clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from client(%s, %s)\n", hostname, port);
    // 여기부터 client -> proxy 시작.
    doit(c_connfd_p);
    Close(connfd);

  }
  
  // printf("%s", user_agent_hdr);
  return 0;
}


/*******************************
 * 한 개의 HTTP 트랜잭션을 처리합니다.
 *******************************/
void doit(int fd)
{
  int is_static;
  int clientfd;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

   /* Read request line and headers */
  Rio_readinitb(&rio, fd); 
  Rio_readlineb(&rio, buf, MAXLINE);                  // 요청 라인을 읽고 분석한다. Tiny는 GET 메소드만 지원함.
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);      // 성공적으로 변환 및 지정된 필드 수를 리턴함. 리턴값은 읽었지만 지정되지 않은 필드는 포함 안됨.

  if (strcasecmp(method, "GET")){
    clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
    return;
  }
  /* 아래부터 본격적인 proxy의 시작 */
  Rio_writen(&rio, buf, strlen(buf));

  read_requesthdrs(&rio);
  /* Parse URI from GET request*/
  is_static = parse_uri(uri, filename, cgiargs);      // 요청이 동적인지, 정적 컨텐츠를 위한 것인지를 위한 플래그 설정
  if (stat(filename, &sbuf) < 0){
    clienterror(fd, filename, "404", "Not found",     // 파일이 디스크 상에 있지 않으면, 에러 메시지를 클라이언트에 보내고 리턴
                "Tiny couldn't find this file");
    return;
  }

  if (is_static)
  { /* Serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))  //검증하기. 일반파일이 아니거나, 파일 읽기권한이 없는 경우 403 에러 안내 실행
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);         // 정적 컨텐츠를 클라이언트에 제공
  }
  else
  { 
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))  // 요청이 동적 컨텐츠에 대한 것이라면 이 파일이 실행 가능한지 검증
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't run the CGI program");
      return;
    }

    serve_dynamic(fd, filename, cgiargs);             // 동적 컨텐츠를 클라이언트에 제공
  }
}

/**********************************
 * HTTP 응답을 응답 라인에 적절한 상태 코드와 상태 메시지를 클라이언트에 보내고,
 * 브라우저 사용자에게 에러를 설명하는 응답 본체에 HTML 파일도 함께 전송
 * ********************************/
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,char *longmsg){
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n",body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body); 

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

/*************************************
 * 요청 헤더 내의 정보를 무시한다.(원본)
 * proxy에서는 요청 헤더 4가지를 살린다.
 * ***********************************/
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  // strcmp는 두 스트링을 비교하고 그 관계를 나타내는 값을 반환
  while (strcmp(buf, "\r\n"))           // 요청 헤더를 종료하는 빈 텍스트 줄
  {
    printf("%s", buf);
    Rio_readlineb(rp, buf, MAXLINE);
  }
  return;
}

/**************************************
 * 정적 컨텐츠를 위한 홈 디렉토리를 명시해주는 함수
 * URI를 파일 이름과 옵션으로 CGI 인자 스트링을 분석한다.
 * ************************************/
int parse_uri(char *uri, char *filename, char *cgiargs){
  char *ptr;

  if (!strstr(uri, "cgi-bin"))        // 정적 컨텐츠를 위한 것이라면
  { /* Static content */
    strcpy(cgiargs, "");              // cgi 인자 스트링을 지우고
    strcpy(filename, ".");            // uri를 ./index.html과 같은 상대 리눅스 경로이름으로 변환
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')  // '/'문자로 끝난다면
      strcat(filename, "home.html");  // 기본 파일 이름을 추가
    return 1;
  }
  else
  { /* Dynamic content */
    ptr = index(uri, '?');            // 동적 컨텐츠를 위한 것이라면
    if (ptr)                          // 모든 CGI 인자들을 추출하고
    {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, "");
    strcpy(filename, ".");            // 나머지 URI 부분을 상대 리눅스 파일 이름으로 변환
    strcat(filename, uri);
    return 0;
  }
}

/**************************************
 * 지역 파일의 내용을 포함하고 있는 본체를 갖는 HTTP 응답을 보낸다.
 * ************************************/
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);     
  sprintf(buf, "HTTP/1.0 200 OK\r\n");  // 클라이언트에 응답 줄과 응답 헤더를 보냄
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client
  요청한 파일의 내용을 연결 식별자 fd로 복사해서 응답 본체로 보냄.
  주의 깊게 봐야 함 */
  srcfd = Open(filename, O_RDONLY, 0);  // 읽기 위해 filename을 open하고 식별자를 얻어옴
  srcp = (char *)Malloc(filesize);      // malloc 기본형. char = 1byte.
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);   // 리눅스 mmap 함수는 요청한 파일을 가상메모리 영역에 매핑함
  // mmap을 호출하면 파일 srcfd의 첫 번째 filesize 바이트 주소 srcp에서 시작하는 사적 읽기-허용 가상메모리 영역으로 매핑함
  Rio_readn(srcfd, srcp, filesize);     // 파일 식별자를 통해 가상 메모리에 파일을 파일사이즈 만큼을 매핑.
  Close(srcfd);                         // 파일을 메모리로 매핑한 후에 더 이상 식별자가 필요없으므로 파일을 닫음(메모리 누수 방지)
  Rio_writen(fd, srcp, filesize);       // 파일을 클라이언트로 전송, 주소 srcp에서 시작하는 filesize 바이트를 클라이언트의 연결 식별자로 복사
  free(srcp);
  // Munmap(srcp, filesize);               // 매핑된 가상메모리 주소를 반환(메모리 누수 방지)
}

/**************************************
 * 파일 이름의 접미어 부분을 검사해서 파일 타입을 결정한다.
 * ************************************/
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "image/mp4");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".css"))
    strcpy(filetype, "text/css");
  else if (strstr(filename, ".js"))
    strcpy(filetype, "application/javascript");
  else if (strstr(filename, ".ico"))
    strcpy(filetype, "image/x-icon");
  else
    strcpy(filetype, "text/plain");
}

/**************************************
 * 클라이언트에 성공을 알려주는 응답 라인을 보내고, 새로운 자식 프로세스를 fork하는 함수
 * ************************************/
// void serve_dynamic(int fd, char *filename, char *cgiargs)
// {
//   char buf[MAXLINE], *emptylist[] = {NULL};

//   /* Return first part of HTTP response */
//   sprintf(buf, "HTTP/1.0 200 OK\r\n");          // 클라이언트에 성공을 알려주는 응답 라인을 보냄
//   Rio_writen(fd, buf, strlen(buf));
//   sprintf(buf, "Server: Tiny Web Server\r\n");
//   Rio_writen(fd, buf, strlen(buf));

//   if (Fork() == 0)                              // 새로운 자식 프로세스를 fork
//   { /* child */
//     /* Real server would set all CGI vars here */
//     setenv("QUERY_STRING", cgiargs, 1);         // 자식은 QUERY_STRING 환경변수를 요청 URI의 CGI 인자들로 초기화함
//     Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to client, 연결 파일 식별자로 자식의 표준 출력을 재지정 */
//     Execve(filename, emptylist, environ); /* Run CGI program, CGI 프로그램을 로드하고 실행 */
//   }
//   Wait(NULL); /* Parent waits for and reaps child, 부모는 자식이 종료되어 정리되는 것을 기다리기 위해 wait 함수에서 블록 */
// }