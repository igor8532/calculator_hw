#include <gtest/gtest.h>

#include "Application.h"
#include "Logger.h" // Чтобы убедиться, что логгер не мешает

#include <iostream>
#include <string>

void runCalculator(const std::string& jsonInput, std::string& stdOutput, std::string& stdError) {
    char* argv[] = {
        const_cast<char*>("calculator_hw"),
        const_cast<char*>(jsonInput.c_str())
    };
    int argc = 2;

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();

    try {
        calculator::Application app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    stdOutput = testing::internal::GetCapturedStdout();
    stdError = testing::internal::GetCapturedStderr();
}

void runCalculatorWithHelp(std::string& stdOutput) {
    char* argv[] = {
        const_cast<char*>("calculator_hw"),
        const_cast<char*>("-h")
    };
    int argc = 2;

    testing::internal::CaptureStdout();
    calculator::Application app;
    app.run(argc, argv);
    stdOutput = testing::internal::GetCapturedStdout();
}

TEST(CalculatorTest, HandlesCorrectAddition) {
    std::string output, error;
    runCalculator(R"({"firstValue":5,"secondValue":3,"operation":"+"})", output, error);
    
    EXPECT_TRUE(output.find("5 + 3 = 8") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesCorrectSubtraction) {
    std::string output, error;
    runCalculator(R"({"firstValue":10,"secondValue":4,"operation":"-"})", output, error);
    
    EXPECT_TRUE(output.find("10 - 4 = 6") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesCorrectDivision) {
    std::string output, error;
    runCalculator(R"({"firstValue":16,"secondValue":4,"operation":"/"})", output, error);
    
    EXPECT_TRUE(output.find("16 / 4 = 4") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesCorrectMultiplication) {
    std::string output, error;
    runCalculator(R"({"firstValue":3,"secondValue":5,"operation":"*"})", output, error);
    
    EXPECT_TRUE(output.find("3 * 5 = 15") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesCorrectPower) {
    std::string output, error;
    runCalculator(R"({"firstValue":4,"secondValue":3,"operation":"^"})", output, error);
    
    EXPECT_TRUE(output.find("4 ^ 3 = 64") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesFactorial) {
    std::string output, error;
    runCalculator(R"({"firstValue":5,"operation":"!"})", output, error);
    
    EXPECT_TRUE(output.find("5! = 120") != std::string::npos);
    EXPECT_TRUE(error.empty());
}

TEST(CalculatorTest, HandlesFactorialWarningForSecondValue) {
    std::string output, error;
    runCalculator(R"({"firstValue":5,"secondValue":10,"operation":"!"})", output, error);
    
    EXPECT_TRUE(output.find("Warning: factorial ignores secondValue") != std::string::npos);
    EXPECT_TRUE(output.find("5! = 120") != std::string::npos);
}

TEST(CalculatorTest, HandlesOverflow) {
    std::string output, error;
    runCalculator(R"({"firstValue":1000000,"secondValue":1000000,"operation":"*"})", output, error);
    
    EXPECT_TRUE(error.find("Overflow") != std::string::npos);
}

TEST(CalculatorTest, HandlesMissingField) {
    std::string output, error;
    runCalculator(R"({"firstValue":5})", output, error);
    
    EXPECT_TRUE(error.find("Missing field: operation") != std::string::npos);
}

TEST(CalculatorTest, HandlesDivisionByZero) {
    std::string output, error;
    runCalculator(R"({"firstValue":10,"secondValue":0,"operation":"/"})", output, error);
    
    EXPECT_TRUE(error.find("Division by zero") != std::string::npos);
}

TEST(CalculatorTest, HandlesNegativeFactorial) {
    std::string output, error;
    runCalculator(R"({"firstValue":-5,"operation":"!"})", output, error);
    
    EXPECT_TRUE(error.find("Factorial is defined only for non-negative integers") != std::string::npos);
}

TEST(CalculatorTest, HandlesUnknownOperation) {
    std::string output, error;
    runCalculator(R"({"firstValue":5,"secondValue":3,"operation":"%"})", output, error);
    
    EXPECT_TRUE(error.find("Unknown operation") != std::string::npos);
}

TEST(CalculatorTest, HandlesZeroToZeroPower) {
    std::string output, error;
    runCalculator(R"({"firstValue":0,"secondValue":0,"operation":"^"})", output, error);
    
    EXPECT_TRUE(error.find("0^0 is mathematically undefined") != std::string::npos);
}

TEST(CalculatorTest, HandlesNegativePower) {
    std::string output, error;
    runCalculator(R"({"firstValue":2,"secondValue":-3,"operation":"^"})", output, error);
    
    EXPECT_TRUE(error.find("Negative power") != std::string::npos);
}

TEST(CalculatorTest, HandlesHelp) {
    std::string output;
    runCalculatorWithHelp(output);
    
    EXPECT_TRUE(output.find("Usage:") != std::string::npos);
    EXPECT_TRUE(output.find("calculator_hw '<json>'") != std::string::npos);
}