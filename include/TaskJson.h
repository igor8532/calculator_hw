#pragma once

#include "Task.h"

#include <nlohmann/json.hpp>

#include <string>

namespace calculator
{

inline void to_json(nlohmann::json& j, const Task& task)
{
    j = nlohmann::json{{"firstValue", task.firstValue},
                       {"secondValue", task.secondValue},
                       {"operation", std::string(1, task.operation)},
                       {"result", task.result},
                       {"status", task.status}};
}

inline void from_json(const nlohmann::json& j, Task& task)
{
    j.at("firstValue").get_to(task.firstValue);
    task.secondValue = j.value("secondValue", 0);
    task.operation = j.at("operation").get<std::string>().at(0);
    task.result = j.value("result", 0);
    task.status = j.value("status", 0);
}

} // namespace calculator
