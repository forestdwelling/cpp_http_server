#include <iostream>
#include <unordered_map>

#include "base.h"
#include "server.h"
#include "database.h"
#include "thread_pool.h"

using namespace std;

class Connection {
public:
    Server *server;
    Client *client;
    Database *database;
    Connection(Server *svr, Client *cli, Database *db): server(svr), client(cli), database(db) {}
};

void handleHttpRequest(void *args) {
    Connection *conn = (Connection *)args;
    HttpRequest request = conn->server->recvHttpRequest(conn->client->clientfd);
    unordered_map<string, string> params = conn->server->parseUrlParam(request.url);
    HttpResponse response("HTTP/1.1", "200", "OK");
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Content-Type"] = "text/html";
    response.headers["Connection"] = "close";
    cout << "===================" << endl;
    if (params["_path"] == "/" || params["_path"] == "/index.html") {
        cout << "Serve index page." << endl;
        string file = readFile("./htdocs/index.html");
        response.headers["Content-Length"] = to_string(file.size());
        response.body = file;
        conn->server->sendHttpResponse(conn->client->clientfd, response);
    } else if (params["_path"] == "/showUser") {
        string data = conn->database->showUser();
        response.headers["Content-Length"] = to_string(data.size());
        response.body = data;
        conn->server->sendHttpResponse(conn->client->clientfd, response);
        cout << "Serve data: " << data << endl;
    } else if (params["_path"] == "/showTransaction") {
        string data = conn->database->showTransaction();
        response.headers["Content-Length"] = to_string(data.size());
        response.body = data;
        conn->server->sendHttpResponse(conn->client->clientfd, response);
        cout << "Serve data: " << data << endl;
    } else if (params["_path"] == "/queryUser") {
        string msg;
        if (params.count("name") != 0 && params.count("passwd")) {
            msg = conn->database->queryUser(params["name"], params["passwd"]);
            response.headers["Content-Length"] = to_string(msg.size());
            response.body = msg;
        }
        conn->server->sendHttpResponse(conn->client->clientfd, response);
        cout << "Query user: " << msg << endl;
    } else if (params["_path"] == "/addUser") {
        if (params.count("name") != 0 && params.count("passwd") != 0 && params.count("identity") != 0) {
            bool flag = conn->database->addUser(params["name"], params["passwd"], params["identity"]);
            string msg = flag?"success":"fail";
            response.headers["Content-Length"] = to_string(msg.size());
            response.body = msg;
            cout << "Add data: " << msg << endl;
        }
        conn->server->sendHttpResponse(conn->client->clientfd, response);
    } else if (params["_path"] == "/addTransaction") {
        if (params.count("uid") != 0 && params.count("t_attr_1") != 0 && params.count("t_attr_2") != 0 && params.count("t_attr_3") != 0) {
            bool flag = conn->database->addTransaction(stoi(params["uid"]), params["t_attr_1"], params["t_attr_2"], params["t_attr_3"]);
            string msg = flag?"success":"fail";
            response.headers["Content-Length"] = to_string(msg.size());
            response.body = msg;
            cout << "Add data: " << msg << endl;
        }
        conn->server->sendHttpResponse(conn->client->clientfd, response);
    } else if (params["_path"] == "/deleteUser") {
        if (params.count("uid") != 0) {
            bool flag = conn->database->deleteUser(stoi(params["uid"]));
            string msg = flag?"success":"fail";
            response.headers["Content-Length"] = to_string(msg.size());
            response.body = msg;
            cout << "Delete data: " << msg << endl;
        }
        conn->server->sendHttpResponse(conn->client->clientfd, response);
    } else if (params["_path"] == "/deleteTransaction") {
        if (params.count("tid") != 0) {
            bool flag = conn->database->deleteTransaction(stoi(params["tid"]));
            string msg = flag?"success":"fail";
            response.headers["Content-Length"] = to_string(msg.size());
            response.body = msg;
            cout << "Delete data: " << msg << endl;
        }
        conn->server->sendHttpResponse(conn->client->clientfd, response);
    } else {
        cout << "Serve not found page." << endl;
        string file = readFile("./htdocs/not_found.html");
        HttpResponse notfound("HTTP/1.1", "404", "NOT FOUND");
        notfound.headers["Content-Type"] = "text/html";
        notfound.headers["Connection"] = "close";
        notfound.headers["Content-Length"] = to_string(file.size());
        notfound.body = file;
        conn->server->sendHttpResponse(conn->client->clientfd, notfound);
    }
    cout << "===================" << endl;
    for (auto iter = conn->server->clientPool.begin(); iter != conn->server->clientPool.end(); iter++) {
        if (iter->clientfd == conn->client->clientfd) {
            conn->server->clientPool.erase(iter);
            break;
        }
    }
}

int main() {
    Server server("0.0.0.0", 80);
    struct epoll_event events[EVENT_NUM];
    ThreadPool pool(MAX_THREAD, EVENT_NUM);
    Database database;
    while (true) {
        int iEventCount = epoll_wait(server.epollfd, events, EVENT_NUM, -1);
        if (iEventCount < 0)
            errorExit("Epoll_wait error.");
        for (int i = 0; i < iEventCount; i++) {
            if (events[i].data.ptr == NULL) {
                server.handleAccept();
                cout << "Accept success." << endl;
            } else {
                cout << "Receive message." << endl;
                Client *client = (Client *)events[i].data.ptr;
                Connection conn(&server, client, &database);
                pool.addTask(handleHttpRequest, (void *)&conn);
                // handleHttpRequest((void *)&conn);
                epoll_ctl(server.epollfd, EPOLL_CTL_DEL, client->clientfd, NULL);
            }
        }
    }
    close(server.listenfd);
    return 0;
}