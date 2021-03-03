#pragma once

#include <exception>
#include <memory>
#include <mysql/mysql.h>
#include <sstream>
#include <string>
#include <vector>

namespace Scorpion {
namespace MySQL {

class Parameter {
public:
    enum ParamType {
        INTEGER = 0,
        STRING = 1,
        BLOB = 2,
        INT64_VECTOR = 3,
        STRING_VECTOR = 4,
        UINT_VECTOR = 5,
    } type;

    union {
        int64_t integer;
        const char *string;
        const std::string *blob;
        const std::vector<uint32_t> *uint_vec;
        const std::vector<int64_t> *int64_vec;
        const std::vector<std::string> *string_vec;
    } data;

public:
    explicit Parameter(int8_t i);
    explicit Parameter(int16_t i);
    explicit Parameter(int32_t i);
    explicit Parameter(uint32_t i);
    explicit Parameter(int64_t i);
    explicit Parameter(uint64_t i);
    explicit Parameter(const char *str);
    explicit Parameter(const std::string &blob);
    explicit Parameter(const std::vector<uint32_t> &vec);
    explicit Parameter(const std::vector<int64_t> &vec);
    explicit Parameter(const std::vector<std::string> &vec);
};

class Column {
public:
    Column(const char *data, unsigned int size);
    ~Column();

public:
    const char *GetData() const;
    unsigned int GetSize() const;
    bool IsNull() const;
    std::string ToString() const;

public:
    template <typename T>
    bool ToValue(T &v) {
        std::istringstream is(ToString());
        is >> v;
        return bool(is);
    }

private:
    const char *_data;
    unsigned int _size;
};

class FreeMySQLResult {
public:
    void operator()(MYSQL_RES *res);
};

class SQLException : public std::exception {
public:
    SQLException(int err, const char *fmt, ...);
    explicit SQLException(MYSQL *mysql);
    ~SQLException() override;

public:
    int Error() const;
    const char *what() const noexcept override;

private:
    int _err;
    char _msg[256];
};

class ResultSet {
public:
    ResultSet();
    ResultSet(std::shared_ptr<MYSQL_RES> result, unsigned int affectedRows, uint64_t lastID);
    ~ResultSet();

public:
    MYSQL_RES *GetRawResult() const;
    unsigned int GetAffectedRows() const;
    unsigned long GetLastID() const;
    unsigned int GetColumns() const;

public:
    bool Next(); // note!
    MYSQL_FIELD *GetFields();
    Column Get(int index) const;
    Column Get(const char *name) const;
    long long GetInt(int index, long long null = 0) const;
    long long GetInt(const char *name, long long null = 0) const;
    std::string GetString(int index) const;
    std::string GetString(const char *name) const;

private:
    std::shared_ptr<MYSQL_RES> _result;
    MYSQL_ROW _row;
    unsigned long *_lengths;
    unsigned int _affectedRows;
    unsigned long _lastID;
    unsigned int _columns;
};

} // namespace MySQL
} // namespace Scorpion