#pragma once

#include <string>

namespace Scorpion {
namespace MySQL {

class MySQLConfig {
public:
    enum ConnMode {
        nil = 0,
        thread_specific = 1,
        pool = 2,
        butt = 3,
    };

public:
    MySQLConfig(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd,
                const std::string &database, const std::string &charset, bool autocommit, ConnMode mode);
    ~MySQLConfig();

public:
    const std::string &GetHost() const;
    unsigned short GetPort() const;
    const std::string &GetUser() const;
    const std::string &GetPasswd() const;
    const std::string &GetDatabase() const;
    const std::string &GetCharset() const;
    bool Autocommit() const;
    ConnMode GetMode() const;

    void SetHost(const std::string &host);
    void SetPort(unsigned short port);
    void SetUser(const std::string &user);
    void SetPasswd(const std::string &passwd);
    void SetDatabase(const std::string &database);
    void SetCharset(const std::string &charset);
    void SetAutocommit(bool autocommit);
    bool SetMode(ConnMode mode);

private:
    std::string _host;
    unsigned short _port;
    std::string _user;
    std::string _passwd;
    std::string _database;
    std::string _charset;

    bool _autocommit;
    ConnMode _mode;
};

} // namespace MySQL
} // namespace Scorpion
