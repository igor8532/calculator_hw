#pragma once

#include "DataBase.h"
#include "Task.h"

namespace calculator
{

class Application
{
  public:
    void run(int argc, char** argv);

    // Общая логика: проверить кэш/БД, при промахе — посчитать и записать.
    // Бросает исключения при ошибках вычисления (как и раньше).
    Task processTask(const Task& request);

  private:
    void getTask(int argc, char** argv);
    void makeCalculate(Task& task) const;
    void printResult() const;
    void printHelp() const;

  private:
    Task task_;
    DataBase dataBase_;
};

} // namespace calculator
