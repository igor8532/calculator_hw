#pragma once

#include "DataBaseConfig.h"
#include "EnvConfig.h"
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
        std::string host{getEnvOr("CALCULATOR_DB_HOST", DB_HOST)};
        std::uint16_t port{getEnvOr("CALCULATOR_DB_PORT",
                                    static_cast<std::uint16_t>(DB_PORT))};
        std::string database{getEnvOr("CALCULATOR_DB_NAME", DB_NAME)};
        std::string username{getEnvOr("CALCULATOR_DB_USER", DB_USERNAME)};
        std::string password{getEnvOr("CALCULATOR_DB_PASSWORD", DB_PASSWORD)};
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