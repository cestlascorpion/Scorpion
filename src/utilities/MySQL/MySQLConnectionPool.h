#pragma once

#include "MySQLConnection.h"

namespace Scorpion {
namespace MySQL {

class ConnectionPool {
public:
    virtual Connection *GetConnection() = 0;
    virtual void PutConnection() = 0;
};

} // namespace MySQL
} // namespace Scorpion