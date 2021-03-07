#include "MySQLFactory.h"
#include "MySQLDriver.h"

namespace Scorpion {
namespace MySQL {

MySQLFactory::Source::Source(const MySQLConfig &config)
    : _config(config) {}

MySQLFactory::Source::~Source() = default;

MySQLFactory::MySQLFactory() {
    if (mysql_library_init(0, nullptr, nullptr) != 0) {
        throw SQLException(-1, "library init fail");
    }
}

MySQLFactory::~MySQLFactory() = default;

MySQLFactory *MySQLFactory::Instance() {
    static MySQLFactory factory;
    return &factory;
}

int MySQLFactory::LoadConfig(const char *path) {
    return 0;
}

Connection *MySQLFactory::GetConnection(const std::string &name) {
    return nullptr;
}

void MySQLFactory::addSource(const std::string &name, const MySQLConfig &config) {}

} // namespace MySQL
} // namespace Scorpion