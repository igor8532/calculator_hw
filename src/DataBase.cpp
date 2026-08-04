#include "DataBase.h"

#include "Logger.h"

#include <algorithm>
#include <stdexcept>

namespace calculator
{

DataBase::DataBase() : connection_(nullptr, &PQfinish)
{
    connect();
    warmUpCache();
}

DataBase::~DataBase()
{
    disconnect();
}

std::string DataBase::makeConnectionString() const
{
    return "host=" + config_.host + " port=" + std::to_string(config_.port) +
           " dbname=" + config_.database + " user=" + config_.username +
           " password=" + config_.password;
}

void DataBase::connect()
{
    connection_ =
        ConnectionPtr(PQconnectdb(makeConnectionString().c_str()), &PQfinish);

    if (!connection_)
    {
        throw std::runtime_error("Failed to allocate PostgreSQL connection");
    }

    if (PQstatus(connection_.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(PQerrorMessage(connection_.get()));
    }

    Logger::getInstance().info("DataBase::connect: Connected to PostgreSQL");
}

void DataBase::disconnect()
{
    connection_.reset();
}

std::string DataBase::makeCacheKey(const Task& task) const
{
    int first = task.firstValue;
    int second = task.secondValue;

    if ((task.operation == '+' || task.operation == '*') && first > second)
    {
        std::swap(first, second);
    }

    if (task.operation == '!')
    {
        return std::to_string(first) + "!";
    }

    return std::to_string(first) + task.operation + std::to_string(second);
}

Task DataBase::makeTask(PGresult* result, int row) const
{
    Task task;

    task.firstValue = std::stoi(PQgetvalue(result, row, 0));

    task.secondValue = std::stoi(PQgetvalue(result, row, 1));

    task.operation = PQgetvalue(result, row, 2)[0];

    task.result = std::stoi(PQgetvalue(result, row, 3));

    task.status = std::stoi(PQgetvalue(result, row, 4));

    return task;
}

void DataBase::warmUpCache()
{
    auto& logger = Logger::getInstance();
    logger.info("DataBase::warmUpCache: Loading records from database");

    ResultPtr resultPtr(
        PQexecParams(
            connection_.get(),
            "SELECT first_value, second_value, operation, result, status "
            "FROM operations;",
            0, nullptr, nullptr, nullptr, nullptr, 0),
        &PQclear);

    if (PQresultStatus(resultPtr.get()) != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(PQerrorMessage(connection_.get()));
    }

    for (int row = 0; row < PQntuples(resultPtr.get()); ++row)
    {
        Task task = makeTask(resultPtr.get(), row);
        cache_.emplace(makeCacheKey(task), task);
    }

    logger.info("DataBase::warmUpCache: Loaded " +
                std::to_string(cache_.size()) + " records");
}

std::optional<Task> DataBase::getRecord(const Task& task) const
{
    auto& logger = Logger::getInstance();

    const auto it = cache_.find(makeCacheKey(task));

    if (it == cache_.end())
    {
        logger.info("DataBase::getRecord: Cache miss");
        return std::nullopt;
    }

    logger.info("DataBase::getRecord: Cache hit");
    return it->second;
}

void DataBase::writeRecord(const Task& task)
{
    auto& logger = Logger::getInstance();
    logger.info("DataBase::writeRecord: Writing record to database");

    const std::string firstValue = std::to_string(task.firstValue);
    const std::string secondValue = std::to_string(task.secondValue);
    const std::string operation(1, task.operation);
    const std::string result = std::to_string(task.result);
    const std::string status = std::to_string(task.status);

    const char* values[]{firstValue.c_str(), secondValue.c_str(),
                         operation.c_str(), result.c_str(), status.c_str()};

    const char* insertSql =
        "INSERT INTO operations "
        "(first_value, second_value, operation, result, status) "
        "VALUES ($1, $2, $3, $4, $5);";

    ResultPtr resultPtr(PQexecParams(connection_.get(), insertSql, 5, nullptr,
                                     values, nullptr, nullptr, 0),
                        &PQclear);

    if (PQresultStatus(resultPtr.get()) != PGRES_COMMAND_OK)
    {
        throw std::runtime_error(PQerrorMessage(connection_.get()));
    }

    cache_.emplace(makeCacheKey(task), task);

    logger.info("DataBase::writeRecord: Record added to cache");
}

} // namespace calculator