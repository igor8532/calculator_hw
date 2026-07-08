#include "Application.h"

#include "libmath.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

namespace calculator
{

void Application::printHelp() const
{
    std::cout << R"(Usage:
  calculator_hw '<json>'

JSON format:
{
  "firstValue": 2,
  "secondValue": 3,
  "operation": "+"
}

Examples:
  calculator_hw '{"firstValue":2,"secondValue":3,"operation":"+"}'
  calculator_hw '{"firstValue":5,"operation":"!"}'
)";
}

void Application::getTask(int argc, char** argv)
{
    if (argc != 2)
    {
        throw std::invalid_argument(
            "Usage: calculator_hw "
            "'{\"firstValue\":2,\"secondValue\":3,\"operation\":\"+\"}'");
    }

    if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")
    {
        printHelp();
        std::exit(EXIT_SUCCESS);
    }

    nlohmann::json data;

    try
    {
        data = nlohmann::json::parse(argv[1]);
    }
    catch (const nlohmann::json::parse_error&)
    {
        throw std::invalid_argument("Invalid JSON format");
    }

    if (!data.contains("firstValue"))
    {
        throw std::invalid_argument("Missing field: firstValue");
    }

    task_.firstValue = data.at("firstValue").get<int>();

    if (!data.contains("operation"))
    {
        throw std::invalid_argument("Missing field: operation");
    }

    const std::string operation = data.at("operation").get<std::string>();

    if (operation.size() != 1)
    {
        throw std::invalid_argument(
            "Operation must contain exactly one character");
    }

    task_.operation = operation[0];

    if (task_.operation == '!')
    {
        if (data.contains("secondValue"))
        {
            std::cout << "Warning: factorial ignores secondValue\n";
        }

        task_.secondValue = 0;
    }
    else
    {
        if (!data.contains("secondValue"))
        {
            throw std::invalid_argument("Missing field: secondValue");
        }

        task_.secondValue = data.at("secondValue").get<int>();
    }
}

void Application::makeCalculate()
{
    task_.status = 0;
    switch (task_.operation)
    {
        case '+':
            task_.status = libmath::addition(task_.firstValue,
                                             task_.secondValue, task_.result);
            break;
        case '-':
            task_.status = libmath::subtraction(
                task_.firstValue, task_.secondValue, task_.result);
            break;
        case '*':
            task_.status = libmath::multiplication(
                task_.firstValue, task_.secondValue, task_.result);
            break;
        case '/':
            task_.status = libmath::division(task_.firstValue,
                                             task_.secondValue, task_.result);
            break;
        case '^':
            task_.status = libmath::power(task_.firstValue, task_.secondValue,
                                          task_.result);
            break;
        case '!':
            task_.status = libmath::factorial(task_.firstValue, task_.result);
            break;
        default:
            task_.status = 1;
            break;
    }

    switch (task_.status)
    {
        case 0:
            return;

        case -1:
            throw std::runtime_error("Division by zero");

        case 1:
            throw std::invalid_argument("Unknown operation");

        case -2:
            throw std::overflow_error("Overflow");

        case -3:
            throw std::invalid_argument("Negative power");

        case -4:
            throw std::domain_error("0^0 is mathematically undefined");

        case -5:
            throw std::domain_error(
                "Factorial is defined only for non-negative integers");

        default:
            throw std::runtime_error("Unknown error");
    }
}

void Application::printResult() const
{
    if (task_.operation == '!')
    {
        std::cout << task_.firstValue << "! = " << task_.result << '\n';
    }
    else
    {
        std::cout << task_.firstValue << " " << task_.operation << " "
                  << task_.secondValue << " = " << task_.result << '\n';
    }
}

void Application::run(int argc, char** argv)
{
    try
    {
        getTask(argc, argv);
        makeCalculate();
        printResult();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

} // namespace calculator