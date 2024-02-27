#include "csapp.h"

// echo 함수는 rio_readlineb 함수가 EOF를 만날 때까지 텍스트 줄을 반복해서 읽고 써줍니다.
void echo(int connfd){                  // 연결 식별자를 받습니다.
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);                // open한 식별자마다 한 번 호출합니다. 
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){    // text line을 안전하게 읽습니다. 
        printf("server received : %d bytes\n", (int)n);
        printf("received message : %s", buf);   // 래퍼 함수로 내부 읽기 버퍼에 복사한 텍스트 라인을 읽어옴
        Rio_writen(connfd, buf, n);             // usrbuf 에서 식별자 fd로 n 바이트를 전송합니다.
    }
}

int main(int argc, char **argv){
    int listenfd, connfd;                       // 듣기 식별자와 연결 식별자
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);                                  // 듣기 식별자
    // port에 연결 요청을 받을 준비가 된 듣기 식별자를 리턴합니다.
    while(1){
        clientlen = sizeof(struct sockaddr_storage);                    // 소켓 주소 구조체
        // 서버는 accept 함수를 호출해서 클라이언트로부터의 연결 요청을 기다립니다.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);       // 연결 식별자
        // 이 소켓 주소체 sa를 대응되는 호스트와 서비스 이름 스트링으로 변환, 이들을 host와 service 버퍼로 복사
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,    
        client_port, MAXLINE, 0);
        // hostname과 port 주소를 출력합니다.
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        Close(connfd);
    }
    exit(0);
}
/*
 * 듣기 식별자는 클라이언트 연결 요청에 대해 끝점으로서의 역할을 합니다.
 한번 생성되며 서버가 살아있는 동안 계속 존재합니다.
 * 연결 식별자는 클라이언트와 서버 사이에 성립된 연결의 끝점입니다.
 서버가 연결 요청을 수락할 때마다 생성되며, 서버가 클라이언트에 서비스하는 동안에만 존재합니다.

 * getinfo 함수는 getaddrinfo 의 역입니다. 이것은 소켓 주소 구조체를 대응되는 호스트와 서비스 이름
 스트링으로 변환합니다. 재진입 가능하고, 프로토콜 독립적입니다.
 sa 인자는 길이 salen 바이트의 소켓 주소 구조체를 가리키고 host 는 hostlen 바이트 길이의 버퍼로,
 service 는 길이 servlen 바이트의 버퍼를 가르킵니다. 만일 getnameinfo 가 0이 아닌 에러코드를
 리턴하면, app은 이것을 gai_strerror 를 호출해서 스트링으로 변환할 수 있습니다.
 만일 우리가 호스트 이름을 원치 않는다면, host를 NULL로 설정하고, hostlen을 0으로 설정할 수 있습니다.
 flag 인자는 비트 마스크를 기본 동작으로 수정합니다.

 ---------------------------------- I / O --------------------------------------------------------
 * RIO I/O (Robust I/O)는 짧은 카운트가 발생할 수 있는 네트워크 프로그램에서 자동 처리하여 안정적이고
 효율적인 I/O를 제공합니다. 두 가지의 서로 다른 종류의 함수를 제공합니다.
 -> 버퍼 없는 입력 및 출력 함수 / 버퍼를 사용하는 입력 함수
*/