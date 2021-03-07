#pragma once

#include "MySQLConfig.h"
#include "MySQLConnection.h"
#include "MySQLConnectionPool.h"
#include <map>
#include <string>

namespace Scorpion {
namespace MySQL {

class MySQLFactory {
public:
    static MySQLFactory *Instance();
    virtual ~MySQLFactory();

    MySQLFactory(const MySQLFactory &) = delete;
    MySQLFactory &operator=(const MySQLFactory &) = delete;

private:
    MySQLFactory();

public:
    int LoadConfig(const char *path);
    Connection *GetConnection(const std::string &name);

private:
    struct Source {
        explicit Source(const MySQLConfig &config);
        ~Source();

        const MySQLConfig &_config;
        std::unique_ptr<ConnectionPool> _pool;
    };

private:
    void addSource(const std::string &name, const MySQLConfig &config);

private:
    std::map<std::string, Source *> _sources;
};

} // namespace MySQL
} // namespace Scorpion