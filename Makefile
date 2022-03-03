main: main.cpp server thread_pool database base
	g++ -o ./bin/main -I /usr/include/cppconn main.cpp ./bin/server ./bin/thread_pool ./bin/database ./bin/base -lpthread -lmysqlcppconn
server: server.h server.cpp
	g++ -o ./bin/server -c server.cpp
thread_pool: thread_pool.h thread_pool.cpp
	g++ -o ./bin/thread_pool -c thread_pool.cpp
database: database.h database.cpp
	g++ -o ./bin/database -I /usr/include/cppconn -c database.cpp
base: base.h base.cpp
	g++ -o ./bin/base -c base.cpp