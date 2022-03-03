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

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include "base.h"
#include "server.h"

using namespace std;

HttpResponse::HttpResponse(string version, string statusCode, string statusText) {
    this->version = version;
    this->statusCode = statusCode;
    this->statusText = statusText;
}

Server::Server(const char *host, int port) {
    struct sockaddr_in name;

    this->listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (this->listenfd == -1)
        errorExit("socket");
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    // name.sin_addr.s_addr = htonl(INADDR_ANY); /* 自动获取当前网卡的有效ip地址 */
    name.sin_addr.s_addr = inet_addr(host);
    
    if (bind(this->listenfd, (struct sockaddr *)&name, sizeof(name)) < 0)
        errorExit("Bind error.");
    /* sin_port=0, bind将随机选择一个未使用的端口号，使用getsockname获取 */
    if (this->port == 0)  /* if dynamically allocating a port */
    {
        socklen_t namelen = sizeof(name);
        if (getsockname(this->listenfd, (struct sockaddr *)&name, &namelen) == -1)
            errorExit("getsockname");
        this->port = ntohs(name.sin_port);
    }
    this->epollfd = epoll_create(MAX_CLIENT);
    if (listen(this->listenfd, BACKLOG) < 0)
        errorExit("Listen error.");
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = NULL;
    epoll_ctl(this->epollfd, EPOLL_CTL_ADD, this->listenfd, &event);
}

Server::~Server() {
    close(this->epollfd);
    close(this->listenfd);
}

Client::Client(int clientfd) {
    this->clientfd = clientfd;
}

Client::~Client() {
    close(this->clientfd);
}

void Server::handleAccept() {
    struct sockaddr_in clientSocketName;
    socklen_t iSize = sizeof(clientSocketName);
    int clientfd = accept(this->listenfd, (struct sockaddr *)&clientSocketName, &iSize);
    if (epollfd == -1)
        errorExit("Accept error.");
    this->clientPool.emplace_back(clientfd);
    struct epoll_event event;
    // The events member of the epoll_event structure is a bit mask
    // composed by ORing together zero or more of the following
    // available event types:
    event.events = EPOLLIN;
    // The data member of the epoll_event structure specifies data that
    // the kernel should save and then return (via epoll_wait(2)) when
    // this file descriptor becomes ready.
    event.data.ptr = &this->clientPool.back();
    epoll_ctl(this->epollfd, EPOLL_CTL_ADD, clientfd, &event);
}

// 发送完再返回
bool Server::sendAll(int clientfd, string &msg) {
    const char *sBuffer = msg.c_str();
    int iRemainLen = msg.size();
    int iSentLen = 0;
    while (iSentLen < msg.size()) {
        int i = send(clientfd, sBuffer+iSentLen, iRemainLen, 0);
        if (i < 0)
            return false;
        iSentLen += i;
        iRemainLen -= i;
    }
    return true;
}

// 接收到sEnd后停止接收，返回去掉sEnd后接收到的字符串
string Server::recvUntil(int clientfd, const char *end) {
    string msg;
    int i = 0;
    int n = strlen(end);
    char c = '\0';
    while (i < n || strcmp(msg.substr(i-n, n).c_str(), end) != 0) {
        recv(clientfd, &c, 1, 0);
        msg.push_back(c);
        i++;
    }
    return msg.substr(0, i-n);
}

unordered_map<string, string> Server::parseUrlParam(string &url) {
    unordered_map<string, string> params;
    int startIdx = url.find("?");
    if (startIdx != string::npos) {
        params["_path"] = url.substr(0, startIdx);
        string sParams = url.substr(startIdx+1, url.size()-(startIdx+1));
        vector<string> vParams = split(sParams, "&");
        for (string &param : vParams) {
            int paramIdx = param.find("=");
            if (paramIdx == string::npos)
                continue;
            params[param.substr(0, paramIdx)] = param.substr(paramIdx+1, param.size()-(paramIdx+1));
        }
    } else {
        params["_path"] = url;
    }
    return params;
}

// 接收并解析Http请求，返回map的key中包括
// Method: 请求方法
// URL: 请求路径资源
// Version: Http协议版本
// Body: 消息主体
// 以及其他header部分
HttpRequest Server::recvHttpRequest(int clientfd) {
    HttpRequest request;
    // parse first line
    string line = this->recvUntil(clientfd, "\r\n");
    cout << line << endl;
    vector<string> vLine = split(line, " ");
    for (auto iter = vLine.begin(); iter != vLine.end(); iter++) {
        cout << *iter << endl;
    }
    request.method = vLine[0];
    request.url = vLine[1];
    request.version = vLine[2];
    // parse headers
    int iContentLength = 0;
    line = this->recvUntil(clientfd, "\r\n");
    while (line != "") {
        vLine = split(line, ": ");
        request.headers.insert(make_pair(vLine[0], vLine[1]));
        if (strcasecmp(vLine[0].c_str(), "Content-Length") == 0) {
            iContentLength = atoi(vLine[1].c_str());
        }
        line = this->recvUntil(clientfd, "\r\n");
    }
    // parse body
    if (iContentLength > 0) {
        string body;
        char c = '\0';
        while (iContentLength > 0) {
            recv(this->epollfd, &c, 1, 0);
            body.push_back(c);
            iContentLength--;
        }
        request.body = body;
    }
    return request;
}

void Server::sendHttpResponse(int clientfd, HttpResponse response){
    string sResponse;
    sResponse.append(response.version+" "+response.statusCode+" "+response.statusText+"\r\n");
    for (auto iter = response.headers.begin(); iter != response.headers.end(); iter++) {
        sResponse.append(iter->first+": "+iter->second+"\r\n");
    }
    sResponse.append("\r\n");
    if (!response.body.empty())
        sResponse.append(response.body);
    this->sendAll(clientfd, sResponse);
}
