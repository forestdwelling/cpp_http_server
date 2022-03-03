#ifndef _SERVER_H_
#define _SERVER_H_

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
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_CLIENT 10000
#define BACKLOG 128
#define EVENT_NUM 64

class HttpRequest {
public:
    std::string method;
    std::string url;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpResponse {
public:
    std::string version;
    std::string statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::string body;
    HttpResponse(std::string version, std::string statusCode, std::string statusText);
};

class Client {
    public:
    int clientfd;
    Client(int clientfd);
    ~Client();
};

class Server {
public:
    int listenfd;
    int epollfd;
    std::list<Client> clientPool;
    int port;
    Server(const char *host, int port);
    ~Server();
    void handleAccept();
    std::string recvUntil(int clientfd, const char *end);
    bool sendAll(int clientfd, std::string &msg);
    std::unordered_map<std::string, std::string> parseUrlParam(std::string &url);
    HttpRequest recvHttpRequest(int clientfd);
    void sendHttpResponse(int clientfd, HttpResponse response);
};

#endif