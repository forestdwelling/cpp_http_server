# CPP HTTP SERVER

A simple HTTP linux server implemented by C++, include the following function:

- HTTP message parsing.
- I/O multiplexing based on epoll.
- Thread pool based on pthread.
- Data persistence based on MySQL.

include the following components:

- main: This component includes the main function, the function executed by thread;
- server: This component wraps basic linux socket function such as send/recv, provides sending, receiving and parsing of HTTP message.
- thread_pool: This component implements a thread pool based on pthread.
- database: This component uses libmysqlcppconn-dev library to access mysql database.