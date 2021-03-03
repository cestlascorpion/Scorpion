#include "MySQLConfig.h"

using namespace std;

namespace Scorpion {
namespace MySQL {

const string &MySQLConfig::GetHost() const {
    return _host;
}

unsigned short MySQLConfig::GetPort() const {
    return _port;
}

const string &MySQLConfig::GetUser() const {
    return _user;
}

const string &MySQLConfig::GetPasswd() const {
    return _passwd;
}

const string &MySQLConfig::GetDatabase() const {
    return _database;
}

const string &MySQLConfig::GetCharset() const {
    return _charset;
}

bool MySQLConfig::Autocommit() const {
    return _autocommit;
}

MySQLConfig::ConnMode MySQLConfig::GetMode() const {
    return _mode;
}

void MySQLConfig::SetHost(const string &host) {
    _host = host;
}

void MySQLConfig::SetPort(unsigned short port) {
    _port = port;
}

void MySQLConfig::SetUser(const string &user) {
    _user = user;
}

void MySQLConfig::SetPasswd(const string &passwd) {
    _passwd = passwd;
}

void MySQLConfig::SetDatabase(const string &database) {
    _database = database;
}

void MySQLConfig::SetCharset(const string &charset) {
    _charset = charset;
}

void MySQLConfig::SetAutocommit(bool autocommit) {
    _autocommit = autocommit;
}

bool MySQLConfig::SetMode(MySQLConfig::ConnMode mode) {
    if (mode > nil && mode < butt) {
        _mode = mode;
        return true;
    }
    return false;
}

MySQLConfig::MySQLConfig(const string &host, unsigned short port, const string &user, const string &passwd,
                         const string &database, const string &charset, bool autocommit, ConnMode mode)
    : _host(host)
    , _port(port)
    , _user(user)
    , _passwd(passwd)
    , _database(database)
    , _charset(charset)
    , _autocommit(autocommit)
    , _mode(mode) {}

MySQLConfig::~MySQLConfig() = default;

} // namespace MySQL
} // namespace Scorpion
