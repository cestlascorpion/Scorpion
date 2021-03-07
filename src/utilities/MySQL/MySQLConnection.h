#pragma once

#include "MySQLDriver.h"
#include <mysql/mysql.h>
#include <string>

namespace Scorpion {
namespace MySQL {

class Statement;

class Connection {
public:
    explicit Connection(unsigned int id);
    ~Connection();

public:
    unsigned int GetID() const;
    bool Connected() const;
    bool Autocommit() const;

    void SetAutoCommit(bool yes);
    void SetCharset(const std::string &charset);

    void Connect(const std::string &user, const std::string &passwd, const std::string &database,
                 const std::string &host, unsigned short port, const std::string &charset = "", bool autocommit = true);
    void Reconnect();
    Statement CreateStatement();
    void Begin();
    void Rollback();
    void Commit();
    void Disconnect();

private:
    const unsigned int _id;
    bool _connected;
    std::string _user;
    std::string _passwd;
    std::string _database;
    std::string _charset;
    std::string _host;
    unsigned short _port;
    bool _autocommit;
    MYSQL _mysql;
};

} // namespace MySQL
} // namespace Scorpion