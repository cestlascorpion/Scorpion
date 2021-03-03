#include "MySQLDriver.h"
#include <cstdio>
#include <cstring>

using namespace std;

namespace Scorpion {
namespace MySQL {

Parameter::Parameter(int8_t i) {
    type = INTEGER;
    data.integer = (int64_t)i;
}

Parameter::Parameter(int16_t i) {
    type = INTEGER;
    data.integer = (int64_t)i;
}

Parameter::Parameter(int32_t i) {
    type = INTEGER;
    data.integer = (int64_t)i;
}

Parameter::Parameter(uint32_t i) {
    type = INTEGER;
    data.integer = (int64_t)i;
}

Parameter::Parameter(int64_t i) {
    type = INTEGER;
    data.integer = i;
}

Parameter::Parameter(uint64_t i) {
    type = INTEGER;
    data.integer = (int64_t)i;
}

Parameter::Parameter(const char *str) {
    type = STRING;
    data.string = str;
}

Parameter::Parameter(const string &blob) {
    type = BLOB;
    data.blob = &blob;
}

Parameter::Parameter(const vector<uint32_t> &vec) {
    type = UINT_VECTOR;
    data.uint_vec = &vec;
}

Parameter::Parameter(const vector<int64_t> &vec) {
    type = INT64_VECTOR;
    data.int64_vec = &vec;
}

Parameter::Parameter(const vector<std::string> &vec) {
    type = STRING_VECTOR;
    data.string_vec = &vec;
}

Column::Column(const char *data, unsigned int size)
    : _data(data)
    , _size(size) {}

Column::~Column() = default;

const char *Column::GetData() const {
    return _data;
}

unsigned int Column::GetSize() const {
    return _size;
}

bool Column::IsNull() const {
    return _data == nullptr;
}

std::string Column::ToString() const {
    if (_data) {
        return string(_data, _size);
    }
    return string();
}

void FreeMySQLResult::operator()(MYSQL_RES *res) {
    if (res != nullptr) {
        mysql_free_result(res);
    }
}

SQLException::SQLException(int err, const char *fmt, ...)
    : _err(err)
    , _msg() {
    va_list var;
    va_start(var, fmt);
    vsnprintf(_msg, sizeof(_msg), fmt, var);
    va_end(var);
}

SQLException::SQLException(MYSQL *mysql)
    : _err((int)mysql_errno(mysql))
    , _msg() {
    strncpy(_msg, mysql_error(mysql), sizeof(_msg));
}

SQLException::~SQLException() = default;

int SQLException::Error() const {
    return _err;
}

const char *SQLException::what() const noexcept {
    return _msg;
}

ResultSet::ResultSet()
    : _result(nullptr)
    , _row(nullptr)
    , _lengths(nullptr)
    , _affectedRows(0)
    , _lastID(0)
    , _columns(0) {}

ResultSet::ResultSet(std::shared_ptr<MYSQL_RES> result, unsigned int affectedRows, uint64_t lastID)
    : _result(result)
    , _row(nullptr)
    , _lengths(nullptr)
    , _affectedRows(affectedRows)
    , _lastID(lastID)
    , _columns(result == nullptr ? 0 : mysql_num_fields(result.get())) {}

ResultSet::~ResultSet() = default;

MYSQL_RES *ResultSet::GetRawResult() const {
    return nullptr;
}

unsigned int ResultSet::GetAffectedRows() const {
    return 0;
}

unsigned long ResultSet::GetLastID() const {
    return 0;
}

unsigned int ResultSet::GetColumns() const {
    return 0;
}

bool ResultSet::Next() {
    if (_result == nullptr) {
        return false;
    }
    _row = mysql_fetch_row(_result.get());
    if (_row != nullptr) {
        _lengths = mysql_fetch_lengths(_result.get());
        return true;
    }
    return false;
}

MYSQL_FIELD *ResultSet::GetFields() {
    if (_result == nullptr) {
        return nullptr;
    }
    return mysql_fetch_fields(_result.get());
}

Column ResultSet::Get(int index) const {
    if (_row == nullptr) {
        throw SQLException(-1, "end of result set");
    }

    auto columns = mysql_num_fields(_result.get());
    if (index <= 0 || (unsigned int)index > columns) {
        throw SQLException(-1, "invalid field index: %d", index);
    }

    return Column(_row[index - 1], (unsigned int)_lengths[index - 1]);
}

Column ResultSet::Get(const char *name) const {
    if (_row == nullptr) {
        throw SQLException(-1, "end of result set");
    }

    auto fields = mysql_fetch_fields(_result.get());
    auto columns = mysql_num_fields(_result.get());
    for (auto i = 0u; i < columns; ++i) {
        if (strcmp(fields[i].name, name) == 0) {
            return Column(_row[i], (unsigned int)_lengths[i]);
        }
    }
    throw SQLException(-1, "invalid field name: %s", name);
}

long long ResultSet::GetInt(int index, long long int null) const {
    long long value;
    if (Get(index).ToValue(value)) {
        return value;
    } else {
        return null;
    }
}

long long ResultSet::GetInt(const char *name, long long int null) const {
    long long value;
    if (Get(name).ToValue(value)) {
        return value;
    } else {
        return null;
    }
}

std::string ResultSet::GetString(int index) const {
    return Get(index).ToString();
}

std::string ResultSet::GetString(const char *name) const {
    return Get(name).ToString();
}

} // namespace MySQL
} // namespace Scorpion