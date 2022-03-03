
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h> //自己加的,exit函数需要
#include <iostream>

using namespace std;

void *sendHttpRequest(void *) {
    static int count = 0;
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char s[100] = "GET / HTTP/1.1\r\n\r\n";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(80);
    len = sizeof(address);
    connect(sockfd, (const sockaddr *)&address, sizeof(address));

    int n = write(sockfd, &s, strlen(s));
    printf("bytes: %d\n", n);
    // n = read(sockfd, &s, 200);
    // printf("string from server: %d", ++count);
    close(sockfd);
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < 100; i++) {
        printf("%d\n", i);
        pthread_t thread;
        pthread_create(&thread, NULL, sendHttpRequest, NULL);
    }
    sleep(1);
    exit(0);
}
