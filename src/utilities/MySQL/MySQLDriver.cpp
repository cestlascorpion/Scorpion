#include "MySQLDriver.h"
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace std;

static string::size_type replace(string &sql, const char *from, const string &to, string::size_type pos) {
    sql.erase(pos, strlen(from));
    sql.insert(pos, to);
    return pos + to.size();
}

namespace Scorpion {
namespace MySQL {

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

string Column::ToString() const {
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

ResultSet::ResultSet(shared_ptr<MYSQL_RES> result, unsigned int affectedRows, uint64_t lastID)
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

string ResultSet::GetString(int index) const {
    return Get(index).ToString();
}

string ResultSet::GetString(const char *name) const {
    return Get(name).ToString();
}

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

Parameter::Parameter(const vector<string> &vec) {
    type = STRING_VECTOR;
    data.string_vec = &vec;
}

Statement::Statement(MYSQL *mysql)
    : _mysql(mysql) {}

Statement::~Statement() = default;

Statement &Statement::operator<<(int i) {
    char buf[22];
    auto len = snprintf(buf, sizeof(buf), "%d", i);
    _sql.append(buf, (unsigned long)len);
    return *this;
}

Statement &Statement::operator<<(unsigned int i) {
    char buf[10];
    auto len = snprintf(buf, sizeof(buf), "%u", i);
    _sql.append(buf, (unsigned long)len);
    return *this;
}

Statement &Statement::operator<<(unsigned long i) {
    char buf[22];
    auto len = snprintf(buf, sizeof(buf), "%ld", i);
    _sql.append(buf, (unsigned long)len);
    return *this;
}

Statement &Statement::operator<<(const char *str) {
    _sql.append(str, strlen(str));
    return *this;
}

Statement &Statement::operator<<(const string &str) {
    _sql.append(str);
    return *this;
}

string Statement::escape(const string &fr) {
    string to;
    if (fr.empty()) {
        to.assign("''");
    } else {
        to.resize(fr.size() * 2 + 3);
        char *buf = (char *)to.data();
        buf[0] = '\'';
        auto len = mysql_real_escape_string(_mysql, buf + 1, fr.data(), fr.size());
        buf[len + 1] = '\'';
        to.resize(len + 2);
    }
    return to;
}

string Statement::escapeNoQuote(const string &fr) {
    string to;
    if (fr.empty()) {
        to.assign("");
    } else {
        to.resize(fr.size() * 2 + 3);
        char *buf = (char *)to.data();
        auto len = mysql_real_escape_string(_mysql, buf, fr.data(), fr.size());
        to.resize(len);
    }
    return to;
}

string Statement::join(const vector<unsigned int> &vec, const char *c) {
    char buf[23];
    string out;
    for (auto i = 0u; i < vec.size(); ++i) {
        auto len = snprintf(buf, sizeof(buf), "%u", vec[i]);
        out.append(buf, (unsigned int)len);
        if (i + 1 < vec.size()) {
            out.append(c);
        }
    }
    return out;
}

string Statement::join(const vector<long> &vec, const char *c) {
    char buf[23];
    string out;
    for (auto i = 0u; i < vec.size(); ++i) {
        auto len = snprintf(buf, sizeof(buf), "%ld", vec[i]);
        out.append(buf, (unsigned int)len);
        if (i + 1 < vec.size()) {
            out.append(c);
        }
    }
    return out;
}

string Statement::join(const vector<string> &vec, const char *c) {
    string out;
    for (auto i = 0u; i < vec.size(); ++i) {
        out.append(escape(vec[i]));
        if (i + 1 < vec.size()) {
            out.append(c);
        }
    }
    return out;
}

void Statement::prepare(const string &sql) {
    _sql = sql;
}

const string &Statement::preview() const {
    return _sql;
}

void Statement::bindParams(const vector<Parameter> &param) {
    int idx;
    char from[4];
    std::string::size_type pos = 0;

    do {
        pos = _sql.find(':', pos);
        if (pos == std::string::npos)
            break;

        idx = atoi(_sql.c_str() + pos + 1);
        assert(idx != 0 && (int)param.size() >= idx);
        snprintf(from, sizeof(from), ":%d", idx);
        const Parameter &p = param[idx - 1];

        switch (p.type) {
        case Parameter::INTEGER: {
            pos = bindInt(pos, from, p.data.integer);
            break;
        }
        case Parameter::STRING: {
            pos = bindString(pos, from, p.data.string, strlen(p.data.string));
            break;
        }
        case Parameter::BLOB: {
            pos = bindString(pos, from, p.data.blob->data(), p.data.blob->size());
            break;
        }
        case Parameter::UINT_VECTOR: {
            std::string vec = join(*(p.data.uint_vec), ",");
            pos = replace(_sql, from, vec, pos);
            break;
        }
        case Parameter::INT64_VECTOR: {
            std::string vec = join(*(p.data.int64_vec), ",");
            pos = replace(_sql, from, vec, pos);
            break;
        }
        case Parameter::STRING_VECTOR: {
            std::string vec = join(*(p.data.string_vec), ",");
            pos = replace(_sql, from, vec, pos);
            break;
        }
        default: {
            break;
        }
        }
    } while (pos < _sql.size());
}

ResultSet Statement::execute() {
    if (mysql_real_query(_mysql, _sql.data(), _sql.size()) != 0) {
        throw SQLException(_mysql);
    }
    auto result = mysql_store_result(_mysql);
    auto affectedRows = mysql_affected_rows(_mysql);
    shared_ptr<MYSQL_RES> res(result, FreeMySQLResult());
    return ResultSet(res, (unsigned int)affectedRows, mysql_insert_id(_mysql));
}

string::size_type Statement::bindInt(string::size_type pos, const char *from, long i) {
    string to;
    to.resize(23);
    char *buf = (char *)to.data();
    auto len = snprintf(buf, 23, "%ld", i);
    to.resize(len);
    return replace(_sql, from, to, pos);
}

string::size_type Statement::bindString(string::size_type pos, const char *from, const char *data, size_t size) {
    string to;
    if (size == 0) {
        to.assign("''");
    } else {
        to.resize(size * 2 + 3);
        char *buf = (char *)to.data();
        buf[0] = '\'';
        auto len = mysql_real_escape_string(_mysql, buf + 1, data, size);
        buf[len + 1] = '\'';
        to.resize(len + 2);
    }
    return replace(_sql, from, to, pos);
}

} // namespace MySQL
} // namespace Scorpion