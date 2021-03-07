#include "MySQLConnection.h"
#include <cassert>

using namespace std;

namespace Scorpion {
namespace MySQL {

Connection::Connection( unsigned int id)
    : _id(id)
    , _connected(false)
    , _port(0)
    , _autocommit(true)
    , _mysql() {
    mysql_thread_init();
}

Connection::~Connection() {
    Disconnect();
    mysql_thread_end();
}

unsigned int Connection::GetID() const {
    return _id;
}

bool Connection::Connected() const {
    return _connected;
}

bool Connection::Autocommit() const {
    return _autocommit;
}

void Connection::SetAutoCommit(bool yes) {
    bool ret = mysql_autocommit(&_mysql, yes ? 1 : 0);
    if (!ret) {
        throw SQLException(&_mysql);
    }
}

void Connection::SetCharset(const string &charset) {
    if (mysql_set_character_set(&_mysql, charset.c_str()) != 0) {
        throw SQLException(&_mysql);
    }
}

void Connection::Connect(const string &user, const string &passwd, const string &database, const string &host,
                         unsigned short port, const string &charset, bool autocommit) {
    if (_connected) {
        return;
    }

    _user = user;
    _passwd = passwd;
    _database = database;
    _host = host;
    _port = port;
    _autocommit = autocommit;
    if (!charset.empty()) {
        _charset = charset;
    }
    assert(mysql_init(&_mysql) != nullptr);
    auto reconnect = 1;
    mysql_options(&_mysql, MYSQL_OPT_RECONNECT, &reconnect);

    if (mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _passwd.c_str(), _database.c_str(), _port, nullptr,
                           0) != &_mysql) {
        throw SQLException(&_mysql);
    }
    mysql_autocommit(&_mysql, _autocommit);
    if (!_charset.empty()) {
        if (mysql_set_character_set(&_mysql, charset.c_str()) != 0) {
            throw SQLException(&_mysql);
        }
    }
    _connected = true;
}

void Connection::Reconnect() {
    Disconnect();
    Connect(_user, _passwd, _database, _host, _port, _charset, _autocommit);
}

Statement Connection::CreateStatement() {
    return Statement(&_mysql);
}

void Connection::Begin() {
    if (mysql_query(&_mysql, "begin") != 0) {
        throw SQLException(&_mysql);
    }
}

void Connection::Rollback() {
    if (mysql_rollback(&_mysql) != 0) {
        throw SQLException(&_mysql);
    }
}

void Connection::Commit() {
    if (mysql_commit(&_mysql) != 0) {
        throw SQLException(&_mysql);
    }
}

void Connection::Disconnect() {
    if (_connected) {
        mysql_close(&_mysql);
        _connected = false;
    }
}

} // namespace MySQL
} // namespace Scorpion