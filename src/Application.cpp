#include "Application.h"

#include "libmath.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace calculator
{

void Application::printHelp() const
{
    printf("Usage: calculator -a <num> -o <op> [-b <num>]\n");
    printf("Options:\n");
    printf("  -a <num>   First operand\n");
    printf("  -o <op>    Operation (+, -, *, /, !, ^)\n");
    printf("  -b <num>   Second operand (for binary operations)\n");
    printf("  -h, --help Show this help\n");
}

void Application::getTask(int argc, char** argv)
{
    int opt = 0;
    bool hasValue1 = false;
    bool hasOperation = false;
    bool hasValue2 = false;

    static struct option longOptions[] = {{"help", no_argument, nullptr, 'h'},
                                          {nullptr, 0, nullptr, 0}};

    optind = 1;

    while ((opt = getopt_long(argc, argv, "ha:o:b:", longOptions, nullptr)) !=
           -1)
    {
        switch (opt)
        {
            case 'h':
                printHelp();
                exit(0);
            case 'a':
                task_.firstValue = atoi(optarg);
                hasValue1 = true;
                break;
            case 'o':
                task_.operation = *(optarg);
                hasOperation = true;
                break;
            case 'b':
                task_.secondValue = atoi(optarg);
                hasValue2 = true;
                break;
            default:
                fprintf(stderr, "Usage: %s -a <num> -o <op> [-b <num>]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!hasValue1 || !hasOperation)
    {
        fprintf(stderr, "Usage: %s -a <num> -o <op> [-b <num>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (task_.operation == '!' && hasValue2)
    {
        fprintf(stderr, "Warning: factorial ignores -b\n");
        task_.secondValue = 0;
    }

    if (task_.operation != '!' && !hasValue2)
    {
        fprintf(stderr, "Error: binary operations need -b\n");
        exit(EXIT_FAILURE);
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
            return;
    }
}

void Application::printResult() const
{
    if (task_.status == 0)
    {
        if (task_.operation == '!')
        {
            printf("%d! = %d\n", task_.firstValue, task_.result);
        }
        else
        {
            printf("%d %c %d = %d\n", task_.firstValue, task_.operation,
                   task_.secondValue, task_.result);
        }
    }
    else if (task_.status == -1)
    {
        printf("Error! Division by zero!\n");
    }
    else if (task_.status == 1)
    {
        printf("Error! Unknown operation!\n");
    }
    else if (task_.status == -2)
    {
        printf("Error! Overflow!\n");
    }
    else if (task_.status == -3)
    {
        printf("Error! Negative power!\n");
    }
    else if (task_.status == -4)
    {
        printf("Error! 0^0 mathematically indefinite\n");
    }
    else if (task_.status == -5)
    {
        printf("To raise to a power, select a non-negative number\n");
    }
    else
    {
        printf("Unknown error\n");
    }
}

void Application::run(int argc, char** argv)
{
    getTask(argc, argv);
    makeCalculate();
    printResult();
}

} // namespace calculator