#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <stdlib.h>
#include <iostream>

#include <mysql_connection.h>
#include <driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

class Database {
public:
    sql::Connection *conn;
    Database();
    ~Database();
    std::string showUser();
    std::string showTransaction();
    std::string queryUser(std::string name, std::string passwd);
    bool addUser(std::string name, std::string passwd, std::string identity);
    bool addTransaction(int uid, std::string t_attr_1, std::string t_attr_2, std::string t_attr_3);
    bool deleteUser(int uid);
    bool deleteTransaction(int tid);
};

// CREATE TABLE user
// (
// uid int AUTO_INCREMENT PRIMARY KEY,
// u_name varchar(255) UNIQUE,
// passwd varchar(255),
// identity varchar(255) NOT NULL
// );

// CREATE TABLE transaction
// (
// tid int AUTO_INCREMENT PRIMARY KEY,
// uid int, 
// t_attr_1 varchar(255),
// t_attr_2 varchar(255),
// t_attr_3 varchar(255)
// );

#endif