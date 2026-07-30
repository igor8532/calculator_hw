#pragma once

#include "DataBaseConfig.h"
#include "Task.h"

#include <libpq-fe.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace calculator
{

class DataBase
{
  public:
    struct Config
    {
        std::string host{DB_HOST};
        std::uint16_t port{DB_PORT};
        std::string database{DB_NAME};
        std::string username{DB_USERNAME};
        std::string password{DB_PASSWORD};
    };

    DataBase();
    ~DataBase();

    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    DataBase(DataBase&&) = delete;
    DataBase& operator=(DataBase&&) = delete;

    void warmUpCache();

    std::optional<Task> getRecord(const Task& task) const;

    void writeRecord(const Task& task);

  private:
    using ConnectionPtr = std::unique_ptr<PGconn, decltype(&PQfinish)>;

    using ResultPtr = std::unique_ptr<PGresult, decltype(&PQclear)>;

    void connect();
    void disconnect();

    std::string makeConnectionString() const;
    std::string makeCacheKey(const Task& task) const;
    Task makeTask(PGresult* result, int row) const;

    Config config_;

    ConnectionPtr connection_;

    std::unordered_map<std::string, Task> cache_;
};

} // namespace calculator