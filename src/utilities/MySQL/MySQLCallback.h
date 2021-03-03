#pragma once

#include <string>

namespace Scorpion {
namespace MySQL {

class ResultSet;
class SQLException;

class Callback {
public:
    Callback() = default;
    virtual ~Callback() = default;

public:
    virtual void onPreview(const std::string &sql) {}
    virtual void onResult(ResultSet &result) {}
    virtual void onException(const SQLException &exception) {}

protected:
    std::string _finalSql;
};

} // namespace MySQL
} // namespace Scorpion