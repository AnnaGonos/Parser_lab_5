#pragma once

#include <string>
#include <vector>

#include "CommandLineOption.h"

namespace ArgumentParser
{

// Парсер аргументов командной строки
class ArgParser
{
public:
    // Конструктор парсера с указанным именем
    explicit ArgParser(std::string name);

    // Добавить целочисленную опцию. Перегрузка с указанием длинного имени
    CommandLineOption& AddIntArgument(std::string longOpt);
    // Добавить целочисленную опцию. Перегрузка с указанием короткого и длинного имен
    CommandLineOption& AddIntArgument(char shortOpt, std::string longOpt);
    // Добавить целочисленную опцию. Перегрузка с указанием длинного имени и описания
    CommandLineOption& AddIntArgument(std::string longOpt, std::string desc);
    // Добавить целочисленную опцию с указанием короткого и длинного имен и описания
    CommandLineOption& AddIntArgument(char shortOpt, std::string longOpt, std::string desc);

    // Добавить строковую опцию
    CommandLineOption& AddStringArgument(std::string longOpt);
    CommandLineOption& AddStringArgument(char shortOpt, std::string longOpt);
    CommandLineOption& AddStringArgument(std::string longOpt, std::string desc);
    CommandLineOption& AddStringArgument(char shortOpt, std::string longOpt, std::string desc);

    // Добавить булеву опцию (флаг)
    CommandLineOption& AddFlag(std::string longOpt);
    CommandLineOption& AddFlag(char shortOpt, std::string longOpt);
    CommandLineOption& AddFlag(std::string longOpt, std::string desc);
    CommandLineOption& AddFlag(char shortOpt, std::string longOpt, std::string desc);

    // Добавить опцию справки
    CommandLineOption& AddHelp(char shortOpt, std::string longOpt, std::string desc);

    // Разобрать аргументы и вернуть успешен ли разбор
    bool Parse(int argc, char** argv);
    bool Parse(const std::vector<std::string>& args);

    // Получить значение флага опции с (длинным) именем longOpt
    bool GetFlag(const std::string& longOpt) const;

    // Получить целочисленное значение опции с (длинным) именем longOpt
    int GetIntValue(const std::string& longOpt) const;

    // Получить целочисленное значение в позиции pos (MultiValue) опции с (длинным) именем longOpt
    int GetIntValue(const std::string& longOpt, size_t pos) const;

    // Получить строковое значение опции с (длинным) именем longOpt
    std::string GetStringValue(const std::string& longOpt) const;

    // Получить строковое значение в позиции pos (MultiValue) опции с (длинным) именем longOpt
    std::string GetStringValue(const std::string& longOpt, size_t pos) const;

    // Запрашивается ли справка
    bool Help();

    // Текст справки
    std::string HelpDescription();

private:
    // Вспомогательные методы

    // Получить объект опции по короткому имени
    CommandLineOption& GetOption(char shortOpt);
    // Получить объект опции по длинному имени
    CommandLineOption& GetOption(const std::string& longOpt);
    // Получить объект опции по длинному имени (перегрузка для константных объектов)
    const CommandLineOption& GetOption(const std::string& longOpt) const;
    // Получить объект позиционного аргумента (аргументов)
    CommandLineOption& GetPositionalArgument();
    // Получить объект опции справки
    const CommandLineOption& GetHelpOption() const;
    // Установить флаг (true) указанного объекта option
    static void SetFlagOption(CommandLineOption& option);
    // Установить значение (value) указанного объекта option
    static void SetValueOption(CommandLineOption& option, const std::string& value);

private:
    const std::string program_name;         // имя
    std::vector<CommandLineOption> options; // опции
};

} // namespace ArgumentParser