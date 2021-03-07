#pragma once

#include "MySQLCallback.h"
#include "MySQLDriver.h"
#include <string>

namespace Scorpion {
namespace MySQL {
namespace Utils {

template <typename T>
struct LexicalCast {
    T operator()(const Column &c) const {
        T v;
        if (c.ToValue(v)) {
            return v;
        } else {
            return T();
        }
    }
};

template <>
struct LexicalCast<std::string> {
    std::string operator()(const Column &c) const {
        return c.ToString();
    }
};

template <std::size_t...>
struct IndexSeq {};

template <std::size_t N, std::size_t... Indices>
struct MakeIndices {
    using type = typename MakeIndices<N - 1, N - 1, Indices...>::type;
};

template <std::size_t... Indices>
struct MakeIndices<0, Indices...> {
    using type = IndexSeq<Indices...>;
};

template <std::size_t N>
using GenerateIndices = typename MakeIndices<N>::type;

template <std::size_t COL_NUMBER, typename... COL_TYPES>
struct ColumnTupleMaker {
    using RT = std::tuple<COL_TYPES...>;

    static RT makeTuple(const ResultSet &resultSet) {
        using IndicesGenerator = utils::GenerateIndices<COL_NUMBER>;
        return makeTuple(resultSet, IndicesGenerator());
    }

    template <std::size_t... Indices>
    static RT makeTuple(const ResultSet &resultSet, utils::IndexSeq<Indices...>) {
        return std::make_tuple(utils::LexicalCast<COL_TYPES>()(resultSet.get(1 + (int)Indices))...);
    }
};

} // namespace Utils

struct CallbackWithErrorAndName : public Callback {
    short _error;
    std::string _msg;
    std::string _name;

    explicit CallbackWithErrorAndName(const std::string &name = "") {
        _name = name;
        _error = 0;
        _msg = "";
    }

    void onResult(ResultSet &result) override = 0;

    void onException(const SQLException &ex) override {
        _error = ex.code();
        _msg = ex.what();
        printf("exception %s [%d,%s]\n", _name.c_str(), ex.code(), ex.what());
    }
};

struct NOPCallback final : public CallbackWithErrorAndName {
    unsigned int _affectedRows;
    unsigned long _lastInsertID;

    explicit NOPCallback(const std::string &name = "")
        : CallbackWithErrorAndName(name) {
        _affectedRows = 0;
        _lastInsertID = 0;
    }

    void onResult(ResultSet &result) override {
        _affectedRows = result.getAffectedRows();
        _lastInsertID = result.getLastId();
    }
};

struct SingleResultSet final : public CallbackWithErrorAndName {
    std::vector<std::string> _result;

    explicit SingleResultSet(const std::string &name = "")
        : CallbackWithErrorAndName(name) {}

    void onResult(ResultSet &result) override {
        _result.clear();
        if (result.next()) {
            auto columns = result.getColumns();
            for (auto pos = 0u; pos < columns; ++pos) {
                _result.push_back(result.getString(pos + 1));
            }
        }
    }
};

struct SingleVector final : public CallbackWithErrorAndName {
    std::vector<std::string> _result;

    explicit SingleVector(const std::string &name = "")
        : CallbackWithErrorAndName(name) {}

    void onResult(ResultSet &result) override {
        _result.clear();
        while (result.next()) {
            _result.push_back(result.getString(1));
        }
    }
};

struct MultiResultSet final : public CallbackWithErrorAndName {
    std::vector<std::vector<std::string>> result_;

    explicit MultiResultSet(const std::string &name = "")
        : CallbackWithErrorAndName(name) {}

    void onResult(ResultSet &result) override {
        result_.clear();
        auto columns = result.getColumns();
        while (result.next()) {
            std::vector<std::string> row;
            for (auto pos = 0u; pos < columns; ++pos) {
                row.push_back(result.getString(pos + 1));
            }
            result_.push_back(row);
        }
    }
};

struct RealResultSet final : public CallbackWithErrorAndName {
public:
    explicit RealResultSet(const std::string &name = "")
        : CallbackWithErrorAndName(name) {}

public:
    void onResult(ResultSet &result) override {
        _result = result;
    }

    const ResultSet &resultSet() const {
        return _result;
    }

    template <typename... COL_TYPES>
    bool next(COL_TYPES &...cols) {
        const std::size_t COL_NUMBER = sizeof...(COL_TYPES);
        if (_result.getColumns() < COL_NUMBER) {
            return false;
        }
        if (_result.next()) {
            using TM = utils::ColumnTupleMaker<COL_NUMBER, COL_TYPES...>;
            std::tie(cols...) = TM::makeTuple(_result);
            return true;
        }
        return false;
    }

    bool empty() const {
        return _result.getAffectedRows() == 0;
    }

private:
    ResultSet _result;
};

template <typename... COL_TYPES>
struct AutoCastCallback final : public CallbackWithErrorAndName {
public:
    using RowType = std::tuple<COL_TYPES...>;
    using ResultType = std::vector<RowType>;
    static const std::size_t COL_NUMBER = sizeof...(COL_TYPES);

    explicit AutoCastCallback(const std::string &name = "")
        : CallbackWithErrorAndName(name) {}

    void onResult(ResultSet &result) override {
        using TM = utils::ColumnTupleMaker<COL_NUMBER, COL_TYPES...>;

        if (result.getColumns() < COL_NUMBER) {
            throw ::SQLException(-10001, "expected column %u, only %u columns in the result set.", COL_NUMBER,
                                 result.getColumns());
        }
        while (result.next()) {
            _result.push_back(TM::makeTuple(result));
        }
    }

    bool empty() const {
        return _result.empty();
    }

    std::size_t rowCount() const {
        return _result.size();
    }

    const ResultType &rows() const {
        return _result;
    }

    const RowType &operator[](int pos) const {
        return _result[pos];
    }

private:
    ResultType _result;
};

} // namespace MySQL
} // namespace Scorpion