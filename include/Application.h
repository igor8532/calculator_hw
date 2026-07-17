#pragma once

#include "Task.h"

namespace calculator
{

class Application
{
  public:
    void run(int argc, char** argv);

  private:
    void getTask(int argc, char** argv);
    void makeCalculate();
    void printResult() const;
    void printHelp() const;

  private:
    Task task_;
};

} // namespace calculator
