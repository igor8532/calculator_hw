#pragma once

namespace calculator
{

struct Task
{
    int firstValue{0};
    int secondValue{0};
    char operation{'\0'};
    int result{0};
    int status{0};
};

} // namespace calculator