#include <stdio.h>
#include <string.h>

/*
strcpy(dest,source)             // dest에 source를 저장
strncpy(dest,source,length)     // source의 스트링에서 length만큼의 문자열을 dest에 저장한다.
strstr(source,찾을부분문자열)    // source에서 ""를 찾아 그 자리를 가르키는 포인터 반환
strchr(source,찾을 한글자)       // source에서 첫번째 ''를 찾아 그 자리를 가르키는 포인터 반환
*/

int main() {
    const char *haystack = "http://www.cmu.edu/hub/index.html HTTP/1.1";
    const char *needle = strstr(haystack,"//");
    char result1[100];
    char result2[100];
    // printf("%zu\n",strlen(haystack)-strlen(needle));
    // strncpy(result,haystack,strlen(haystack)-strlen(needle));
    strcpy(result1, haystack+(strlen(haystack)-strlen(needle)+2)); // 앞 http:// 분리하기
    char * path = strchr(haystack, '/');
    strncpy(result2, result1, strlen(path));
    // void *ptr = strstr(haystack, "//");
    // char host_ptr[100]
    if (result1 != NULL) {
        printf("부분 문자열이 발견되었습니다: %s\n", result1);
        printf("부분 문자열이 발견되었습니다: %s\n", result2);
    } else {
        printf("부분 문자열이 발견되지 않았습니다.\n");
    }

    

    return 0;
}

{
    const char *haystack = "http://www.cmu.edu/hub/index.html HTTP/1.1";
    char host[100], port[100], path[100];
    const char *needle;
    // 호스트(host) 추출
    needle = strstr(haystack, "//");
    if (needle != NULL) {
        needle += 2;
        const char *host_end = strchr(needle, ':');
        if (host_end == NULL) {
            host_end = strchr(needle, '/');
        }
        if (host_end == NULL) {
            printf("올바른 URL 형식이 아닙니다.\n");
            return 1;
        }
        strncpy(host, needle, host_end - needle);
        host[host_end - needle] = '\0';
        printf("Host: %s\n", host);

        // 포트(port) 추출
        if (*host_end == ':') {
            needle = host_end + 1;
            const char *port_end = strchr(needle, '/');
            if (port_end == NULL) {
                printf("올바른 URL 형식이 아닙니다.\n");
                return 1;
            }
            strncpy(port, needle, port_end - needle);
            port[port_end - needle] = '\0';
            printf("Port: %s\n", port);
        } else {
            printf("Port: 80 (기본값)\n");
        }

        // 경로(path) 추출
        needle = strchr(host_end, '/');
        if (needle == NULL) {
            printf("올바른 URL 형식이 아닙니다.\n");
            return 1;
        }
        const char *path_end = strchr(needle, ' '); // 버전 정보 이전까지의 문자열을 찾음
        if (path_end == NULL) {
            printf("올바른 URL 형식이 아닙니다.\n");
            return 1;
        }
        strncpy(path, needle, path_end - needle);
        path[path_end - needle] = '\0';
        printf("Path: %s\n", path);
    } else {
        printf("올바른 URL 형식이 아닙니다.\n");
        return 1;
    }

    return 0;
}
