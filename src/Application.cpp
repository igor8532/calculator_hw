#include "Application.h"

#include "Logger.h"
#include "libmath.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

namespace calculator
{

void Application::printHelp() const
{
    auto& logger = Logger::getInstance();
    std::cout << R"(
    Usage:
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

    logger.info("Application::printHelp: Help displayed");
}

void Application::getTask(int argc, char** argv)
{
    auto& logger = Logger::getInstance();
    logger.debug("Application::getTask: Parsing command line arguments");

    if (argc != 2)
    {
        logger.error("Application::getTask: Invalid number of arguments");
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
        logger.debug("Application::getTask: JSON parsed successfully");
    }
    catch (const nlohmann::json::parse_error& e)
    {
        logger.error("Application::getTask: Failed to parse JSON: " +
                     std::string(e.what()));
        throw std::invalid_argument("Invalid JSON format");
    }

    if (!data.contains("firstValue"))
    {
        logger.error("Application::getTask: Missing field: firstValue");
        throw std::invalid_argument("Missing field: firstValue");
    }

    task_.firstValue = data.at("firstValue").get<int>();
    logger.debug("Application::getTask: firstValue:" +
                 std::to_string(task_.firstValue));

    if (!data.contains("operation"))
    {
        logger.error("Application::getTask: Missing field: operation");
        throw std::invalid_argument("Missing field: operation");
    }

    const std::string operation = data.at("operation").get<std::string>();

    if (operation.size() != 1)
    {
        logger.error("Application::getTask: Invalid operation length:" +
                     std::to_string(operation.size()));
        throw std::invalid_argument(
            "Operation must contain exactly one character");
    }

    task_.operation = operation[0];
    logger.debug("Application::getTask: operation: " +
                 std::string(1, task_.operation));

    if (task_.operation == '!')
    {
        if (data.contains("secondValue"))
        {
            logger.warn("Application::getTask: Factorial ignores secondValue");
            std::cout << "Warning: factorial ignores secondValue\n";
        }

        task_.secondValue = 0;
    }
    else
    {
        if (!data.contains("secondValue"))
        {
            logger.error("Application::getTask: Missing field: secondValue");
            throw std::invalid_argument("Missing field: secondValue");
        }

        task_.secondValue = data.at("secondValue").get<int>();
        logger.debug("Application::getTask: secondValue: " +
                     std::to_string(task_.secondValue));
    }
    logger.info("Application::getTask: JSON parsed successfully");
}

void Application::makeCalculate()
{
    auto& logger = Logger::getInstance();
    logger.debug("Application::makeCalculate: Starting calculation");

    task_.status = 0;
    switch (task_.operation)
    {
        case '+':
            task_.status = libmath::addition(task_.firstValue,
                                             task_.secondValue, task_.result);
            logger.debug("Application::makeCalculate: Addition: " +
                         std::to_string(task_.firstValue) + " + " +
                         std::to_string(task_.secondValue) + " = " +
                         std::to_string(task_.result));
            break;

        case '-':
            task_.status = libmath::subtraction(
                task_.firstValue, task_.secondValue, task_.result);
            logger.debug("Application::makeCalculate: Subtraction: " +
                         std::to_string(task_.firstValue) + " - " +
                         std::to_string(task_.secondValue) + " = " +
                         std::to_string(task_.result));
            break;

        case '*':
            task_.status = libmath::multiplication(
                task_.firstValue, task_.secondValue, task_.result);
            logger.debug("Application::makeCalculate: Multiplication: " +
                         std::to_string(task_.firstValue) + " * " +
                         std::to_string(task_.secondValue) + " = " +
                         std::to_string(task_.result));
            break;

        case '/':
            task_.status = libmath::division(task_.firstValue,
                                             task_.secondValue, task_.result);
            logger.debug("Application::makeCalculate: Division: " +
                         std::to_string(task_.firstValue) + " / " +
                         std::to_string(task_.secondValue) + " = " +
                         std::to_string(task_.result));
            break;

        case '^':
            task_.status = libmath::power(task_.firstValue, task_.secondValue,
                                          task_.result);
            logger.debug("Application::makeCalculate: Power: " +
                         std::to_string(task_.firstValue) + " ^ " +
                         std::to_string(task_.secondValue) + " = " +
                         std::to_string(task_.result));
            break;

        case '!':
            task_.status = libmath::factorial(task_.firstValue, task_.result);
            logger.debug("Application::makeCalculate: Factorial: " +
                         std::to_string(task_.firstValue) +
                         "! = " + std::to_string(task_.result));
            break;

        default:
            task_.status = 1;
            logger.error("Application::makeCalculate: Unknown operation: " +
                         std::string(1, task_.operation));
            break;
    }

    switch (task_.status)
    {
        case 0:
            logger.info("Application::makeCalculate: Calculation completed "
                        "successfully");
            return;

        case -1:
            logger.error("Application::makeCalculate: Division by zero");
            throw std::runtime_error("Division by zero");

        case 1:
            logger.error("Application::makeCalculate: Unknown operation");
            throw std::invalid_argument("Unknown operation");

        case -2:
            logger.error("Application::makeCalculate: Overflow");
            throw std::overflow_error("Overflow");

        case -3:
            logger.error("Application::makeCalculate: Negative power");
            throw std::invalid_argument("Negative power");

        case -4:
            logger.error(
                "Application::makeCalculate: 0^0 is mathematically undefined");
            throw std::domain_error("0^0 is mathematically undefined");

        case -5:
            logger.error("Application::makeCalculate: Factorial is defined "
                         "only for non-negative integers");
            throw std::domain_error(
                "Factorial is defined only for non-negative integers");

        default:
            logger.error("Application::makeCalculate: Unknown error: " +
                         std::to_string(task_.status));
            throw std::runtime_error("Unknown error");
    }
}

void Application::printResult() const
{
    auto& logger = Logger::getInstance();

    if (task_.operation == '!')
    {
        std::cout << task_.firstValue << "! = " << task_.result << '\n';
        logger.info("Application::printResult: Result: " +
                    std::to_string(task_.firstValue) +
                    "! = " + std::to_string(task_.result));
    }
    else
    {
        std::cout << task_.firstValue << " " << task_.operation << " "
                  << task_.secondValue << " = " << task_.result << '\n';
        logger.info("Application::printResult: Result: " +
                    std::to_string(task_.firstValue) + " " +
                    std::string(1, task_.operation) + " " +
                    std::to_string(task_.secondValue) + " = " +
                    std::to_string(task_.result));
    }
}

void Application::run(int argc, char** argv)
{
    auto& logger = Logger::getInstance();
    logger.debug("Application::run: started");

    getTask(argc, argv);
    makeCalculate();
    printResult();

    logger.debug("Application::run: completed");
}

} // namespace calculator