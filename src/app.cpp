#include "app.h"

#include "libmath.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace
{

void printHelp()
{
    printf("Usage: calculator -a <num> -o <op> [-b <num>]\n");
    printf("Options:\n");
    printf("  -a <num>   First operand\n");
    printf("  -o <op>    Operation (+, -, *, /, !, ^)\n");
    printf("  -b <num>   Second operand (for binary operations)\n");
    printf("  -h, --help Show this help\n");
}

struct Task
{
    int value1;
    int value2;
    char operation;
    int result;
    int status;
};

void makeTask(int argc, char** argv, Task& task)
{
    int opt;
    bool hasValue1 = false;
    bool hasOperation = false;
    bool hasValue2 = false;

    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};

    optind = 1;

    while ((opt = getopt_long(argc, argv, "ha:o:b:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'h':
                printHelp();
                exit(0);
            case 'a':
                task.value1 = atoi(optarg);
                hasValue1 = true;
                break;
            case 'o':
                task.operation = *(optarg);
                hasOperation = true;
                break;
            case 'b':
                task.value2 = atoi(optarg);
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
        fprintf(stderr, "Usage: %s -a <num> -o <op> [-b <num>]\n",
		argv[0]); 
        exit(EXIT_FAILURE);
    }

    if (task.operation == '!' && hasValue2)
    {
        fprintf(stderr, "Warning: factorial ignores -b\n");
        task.value2 = 0;
    }

    if (task.operation != '!' && !hasValue2)
    {
        fprintf(stderr, "Error: binary operations need -b\n");
        exit(EXIT_FAILURE);
    }
}

void makeCalculate(Task& task)
{
    task.status = 0;
    switch (task.operation)
    {
        case '+':
            task.status =
                libmath::addition(task.value1, task.value2, task.result);
            break;
        case '-':
            task.status =
                libmath::subtraction(task.value1, task.value2, task.result);
            break;
        case '*':
            task.status =
                libmath::multiplication(task.value1, task.value2, task.result);
            break;
        case '/':
            task.status =
                libmath::division(task.value1, task.value2, task.result);
            break;
        case '^':
            task.status = libmath::power(task.value1, task.value2, task.result);
            break;
        case '!':
            task.status = libmath::factorial(task.value1, task.result);
            break;
        default:
            task.status = 1;
            return;
    }
}

void printResult(Task task)
{
    if (task.status == 0)
    {
        if (task.operation == '!')
        {
            printf("%d! = %d\n", task.value1, task.result);
        }
        else
        {
            printf("%d %c %d = %d\n", task.value1, task.operation, task.value2,
                   task.result);
        }
    }
    else if (task.status == -1)
    {
        printf("Error! Division by zero!\n");
    }
    else if (task.status == 1)
    {
        printf("Error! Unknown operation!\n");
    }
    else if (task.status == -2)
    {
        printf("Error! Overflow!\n");
    }
    else if (task.status == -3)
    {
        printf("Error! Negative power!\n");
    }
    else if (task.status == -4)
    {
        printf("Error! 0^0 mathematically indefinite\n");
    }
    else if (task.status == -5)
    {
        printf("To raise to a power, select a non-negative number\n");
    }
    else
    {
        printf("Unknown error\n");
    }
}

} // namespace

namespace app
{

void applicationRun(int argc, char** argv)
{
    Task task;
    makeTask(argc, argv, task);
    makeCalculate(task);
    printResult(task);
}

} // namespace app
