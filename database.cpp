#include <stdlib.h>
#include <iostream>

#include <mysql_connection.h>
#include <driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

#include "database.h"
#include "base.h"

using namespace std;
using namespace sql;

Database::Database() {
    string host = "localhost";
    string user = "debian-sys-maint";
    string password = "RsTG0zfeDF0sYFFx";
    sql::Driver *driver;
    driver = get_driver_instance();
    this->conn = driver->connect("tcp://"+host+":3306", user, password);
    this->conn->setSchema("lawyer_office");
}

Database::~Database() {
    delete this->conn;
}

string Database::showUser() {
    string data;
    sql::Statement *stmt = this->conn->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM user");
    while (res->next()) {
        data.append(to_string(res->getInt(1))+" ");
        data.append(res->getString(2)+" ");
        data.append(res->getString(3)+" ");
        data.append(res->getString(4)+"<br>");
    }
    delete stmt;
    delete res;
    return data;
}

string Database::showTransaction() {
    string data;
    sql::Statement *stmt = this->conn->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM transaction");
    while (res->next()) {
        data.append(to_string(res->getInt(1))+" ");
        data.append(to_string(res->getInt(2))+" ");
        data.append(res->getString(3)+" ");
        data.append(res->getString(4)+" ");
        data.append(res->getString(5)+"<br>");
    }
    delete stmt;
    delete res;
    return data;
}

string Database::queryUser(string name, string passwd) {
    string identity;
    string sql = "SELECT identity FROM user WHERE u_name=? AND passwd=?";
    sql::PreparedStatement *prep_stmt = this->conn->prepareStatement(sql);
    prep_stmt->setString(1, name);
    prep_stmt->setString(2, passwd);
    bool flag = true;
    try {
        sql::ResultSet *res = prep_stmt->executeQuery();
        if (res->next()) {
            identity = res->getString(1);
        } else {
            identity = "";
        }
        delete res;
    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
        identity = "";
    }
    delete prep_stmt;
    return identity;
}

bool Database::addUser(string name, string passwd, string identity) {
    string sql = "INSERT INTO user(u_name, passwd, identity) VALUES(?, ?, ?)";
    sql::PreparedStatement *prep_stmt = this->conn->prepareStatement(sql);
    prep_stmt->setString(1, name);
    prep_stmt->setString(2, passwd);
    prep_stmt->setString(3, identity);
    bool flag = true;
    try {
        prep_stmt->execute();
    } catch (sql::SQLException &e) {
        flag = false;
    }
    delete prep_stmt;
    return flag;
}

bool Database::addTransaction(int uid, string t_attr_1, string t_attr_2, string t_attr_3) {
    string sql = "INSERT INTO transaction(uid, t_attr_1, t_attr_2, t_attr_3) VALUES(?, ?, ?, ?)";
    sql::PreparedStatement *prep_stmt = this->conn->prepareStatement(sql);
    prep_stmt->setInt(1, uid);
    prep_stmt->setString(2, t_attr_1);
    prep_stmt->setString(3, t_attr_2);
    prep_stmt->setString(4, t_attr_3);
    bool flag = true;
    try {
        prep_stmt->execute();
    } catch (sql::SQLException &e) {
        flag = false;
    }
    delete prep_stmt;
    return flag;
}

bool Database::deleteUser(int uid) {
    string sql = "DELETE FROM product WHERE uid=?";
    sql::PreparedStatement *prep_stmt = this->conn->prepareStatement(sql);
    prep_stmt->setInt(1, uid);
    bool flag = true;
    try {
        prep_stmt->execute();
    } catch (sql::SQLException &e) {
        flag = false;
    }
    delete prep_stmt;
    return flag;
}

bool Database::deleteTransaction(int tid) {
    string sql = "DELETE FROM transaction WHERE tid=?";
    sql::PreparedStatement *prep_stmt = this->conn->prepareStatement(sql);
    prep_stmt->setInt(1, tid);
    bool flag = true;
    try {
        prep_stmt->execute();
    } catch (sql::SQLException &e) {
        flag = false;
    }
    delete prep_stmt;
    return flag;
}

// int main(void)
// {
//     Database db;
//     // db.addUser("admin", "123456", "admin");
//     // cout << db.showUser() << endl;
//     // db.addTransaction(1, "test", "test", "test");
//     // cout << db.showTransaction() << endl;
//     db.addTransaction(2, "test", "test", "test");
//     db.deleteTransaction(1);
//     return 0;
// }