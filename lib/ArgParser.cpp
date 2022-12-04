#include "ArgParser.h"

#include <utility>
#include <algorithm>
#include <sstream>


namespace ArgumentParser
{

ArgParser::ArgParser(std::string name)
        : program_name(std::move(name))
{}

CommandLineOption& ArgParser::AddIntArgument(std::string longOpt)
{
    return AddIntArgument({}, std::move(longOpt), {}); // без короткого имени и описания
}

CommandLineOption& ArgParser::AddIntArgument(char shortOpt, std::string longOpt)
{
    return AddIntArgument(shortOpt, std::move(longOpt), {}); // без описания
}

CommandLineOption& ArgParser::AddIntArgument(std::string longOpt, std::string desc)
{
    return AddIntArgument({}, std::move(longOpt), std::move(desc)); // без короткого имени
}

CommandLineOption& ArgParser::AddIntArgument(char shortOpt, std::string longOpt, std::string desc)
{
    // добавляем новую опцию для целочисленных значений
    options.emplace_back(OptionType::IntegerOption, shortOpt, std::move(longOpt), std::move(desc));
    return options.back(); // возвращаем ссылку на добавленную опцию
}

// аналогично со строковыми опциями

CommandLineOption& ArgParser::AddStringArgument(std::string longOpt)
{
    return AddStringArgument({}, std::move(longOpt), {});
}

CommandLineOption& ArgParser::AddStringArgument(char shortOpt, std::string longOpt)
{
    return AddStringArgument(shortOpt, std::move(longOpt), {});
}

CommandLineOption& ArgParser::AddStringArgument(std::string longOpt, std::string desc)
{
    return AddStringArgument({}, std::move(longOpt), std::move(desc));
}

CommandLineOption& ArgParser::AddStringArgument(char shortOpt, std::string longOpt, std::string desc)
{
    options.emplace_back(OptionType::StringOption, shortOpt, std::move(longOpt), std::move(desc));
    return options.back();
}

// аналогично с опциями-флагами

CommandLineOption& ArgParser::AddFlag(std::string longOpt)
{
    return AddFlag({}, std::move(longOpt), std::string{});
}

CommandLineOption& ArgParser::AddFlag(char shortOpt, std::string longOpt)
{
    return AddFlag(shortOpt, std::move(longOpt), {});
}

CommandLineOption& ArgParser::AddFlag(std::string longOpt, std::string desc)
{
    return AddFlag({}, std::move(longOpt), std::move(desc));
}

CommandLineOption& ArgParser::AddFlag(char shortOpt, std::string longOpt, std::string desc)
{
    options.emplace_back(OptionType::FlagOption, shortOpt, std::move(longOpt), std::move(desc));
    return options.back();
}

// и опцией-справкой
CommandLineOption& ArgParser::AddHelp(char shortOpt, std::string longOpt, std::string desc)
{
    options.emplace_back(OptionType::HelpOption, shortOpt, std::move(longOpt), std::move(desc));
    return options.back();
}

bool ArgParser::Parse(int argc, char** argv)
{ // используем перегруженный метод
    return Parse(std::vector<std::string>(argv, argv + argc));
}

bool ArgParser::Parse(const std::vector<std::string>& args)
{
    if (args.empty()) // нет аргументов (должен быть как минимум один - имя файла самой программы)
        return false;

    try
    {
        // проход по аргументам (пропускаем первый - название программы)
        for (size_t argIndex = 1; argIndex < args.size(); ++argIndex)
        {
            const auto& arg = args[argIndex]; // текущий аргумент
            if (arg.empty()) // аргумент не должен быть пустой
                return false;

            if (arg[0] == '-') // начало опции
            {
                if (arg.size() < 2) // некорректная опция
                    return false;

                auto eq_pos = arg.find('='); // позиция символа '=' в текущем аргументе
                if (eq_pos == std::string::npos) // если '=' не найден
                    eq_pos = arg.size(); // установим на конец текущей опции

                if (eq_pos == arg.size() - 1) // если '=' - последний символ опции
                    return false; // то, опция некорректна

                if (arg[1] == '-') // длинная опция (начинается с "--")
                {
                    if (arg.size() < 3) // после тире должно быть что-то еще
                        return false;

                    const auto longOptName = arg.substr(2, eq_pos - 2); // имя опции (без "--" до '=')
                    auto& opt = GetOption(longOptName); // получаем объект опции для указанного имени
                    if (eq_pos == arg.size()) // если '=' отсутствует,
                    { // значит текущая опция - флаг
                        SetFlagOption(opt); // устанавливаем его значение (true, так как флаг указан)
                        if (opt.GetType() == OptionType::HelpOption) // если был запрос на справку,
                            return true; // успешно завершаем разбор аргументов (требуется только вывод справки)
                    }
                    else // иначе (есть '=')
                    { // устанавливаем для текущей опции значение, указанное после '='
                        SetValueOption(opt, arg.substr(eq_pos + 1));
                    }
                }
                else // иначе, короткая опция (опции)
                {
                    // сколько из них - флаги, например,
                    // -ас - два флага (а и с)
                    // -асх=1 - два флага и целочисленное значение 1 для опции х
                    auto flagCount = eq_pos == arg.size() ? eq_pos : eq_pos - 2;
                    for (size_t j = 1; j < flagCount; ++j) // перебираем флаги
                    {
                        const auto shortOptName = arg[j]; // короткое имя текущего флага
                        auto& opt = GetOption(shortOptName); // получаем опцию по этому имени
                        SetFlagOption(opt); // устанавливаем флаг
                        if (opt.GetType() == OptionType::HelpOption) // если был запрос на справку,
                            return true; // успешно завершаем разбор аргументов (требуется только вывод справки)
                    }

                    if (flagCount != eq_pos) // если кроме флагов, была другая опция (как х в примере выше) (возможна только одна)
                    {
                        const auto shortOptName = arg[eq_pos - 1]; // короткое имя
                        auto& opt = GetOption(shortOptName); // соответствующая опция
                        SetValueOption(opt, arg.substr(eq_pos + 1)); // устанавливаем значение
                    }
                }
            }
            else // иначе, аргумент начинается не с '-', значит все последующие аргументы - позиционные
            {
                auto& opt = GetPositionalArgument(); // ищем опцию для позиционных аргументов
                for (; argIndex < args.size(); ++argIndex) // перебираем все оставшиеся аргументы
                    SetValueOption(opt, args[argIndex]); // добавляем значения
            }
        }
    }
    catch (std::exception&)
    {
        return false; // произошла ошибка - опции некорректны
    }
    // проверяем, что все опции корректны, и возвращаем результат проверки
    return std::all_of(options.begin(), options.end(), [](const auto& opt) { return opt.IsValid(); });
}

int ArgParser::GetIntValue(const std::string& longOpt) const
{
    return GetOption(longOpt).GetInt(); // получение целочисленного значения опции по ее имени
}

int ArgParser::GetIntValue(const std::string& longOpt, size_t pos) const
{
    return GetOption(longOpt).GetInt(pos); // получение целочисленного значения в позиции pos MultiValue опции по ее имени
}

std::string ArgParser::GetStringValue(const std::string& longOpt) const
{
    return GetOption(longOpt).GetString(); // получение строкового значения опции по ее имени
}

std::string ArgParser::GetStringValue(const std::string& longOpt, size_t pos) const
{
    return GetOption(longOpt).GetString(pos); // получение строкового значения в позиции pos MultiValue опции по ее имени
}

CommandLineOption& ArgParser::GetOption(char shortOpt)
{
    // ищем опцию по ее короткому имени среди всех опций
    const auto it = std::find_if(options.begin(), options.end(), [&shortOpt](const auto& opt){
        return opt.GetShortOption() == shortOpt;
    });
    if (it == options.end()) // не найдено - ошибка
        throw std::logic_error(std::string{"No option named "} + shortOpt);
    return *it; // возвращаем ссылку на опцию
}

CommandLineOption& ArgParser::GetOption(const std::string& longOpt)
{
    // используем перегруженный константный метод
    return const_cast<CommandLineOption&>(static_cast<const ArgParser*>(this)->GetOption(longOpt));
}

const CommandLineOption& ArgParser::GetOption(const std::string& longOpt) const
{
    // ищем опцию по ее длинному имени среди всех опций
    const auto it = std::find_if(options.begin(), options.end(), [&longOpt](const auto& opt){
        return opt.GetLongOption() == longOpt;
    });
    if (it == options.end()) // не найдено - ошибка
        throw std::logic_error("No option named " + longOpt);
    return *it; // возвращаем ссылку на опцию
}

CommandLineOption& ArgParser::GetPositionalArgument()
{
    // ищем опцию позиционных аргументов среди всех опций
    const auto it = std::find_if(options.begin(), options.end(), [](const auto& opt){
        return opt.IsPositional();
    });
    if (it == options.end()) // не найдено - ошибка
        throw std::logic_error("No positional option");
    return *it; // возвращаем ссылку на опцию
}

const CommandLineOption& ArgParser::GetHelpOption() const
{
    // ищем опцию справки среди всех опций
    const auto it = std::find_if(options.begin(), options.end(), [](const auto& opt){
        return opt.GetType() == OptionType::HelpOption;
    });
    if (it == options.end()) // не найдено - ошибка
        throw std::logic_error("No help option");
    return *it; // возвращаем ссылку на опцию
}

void ArgParser::SetFlagOption(CommandLineOption& option)
{
    auto type = option.GetType();
    if (type != OptionType::FlagOption && type != OptionType::HelpOption)
        throw std::logic_error("No help option");
    option.SetValue(true); // установка флага
}

void ArgParser::SetValueOption(CommandLineOption& option, const std::string& value)
{
    // установка значения
    const auto type = option.GetType();
    if (type == OptionType::IntegerOption) // для целого числа
        option.SetValue(std::stoi(value));
    else if (type == OptionType::StringOption) // для строки
        option.SetValue(value);
    else // другие типы не поддерживают операцию - ошибка
        throw std::logic_error("Wrong option type");
}

bool ArgParser::GetFlag(const std::string& longOpt) const
{
    return GetOption(longOpt).GetFlag(); // значение флага по его длинному имени
}

bool ArgParser::Help()
{
    return GetHelpOption().GetFlag(); // опция справки - специальный тип флага; возвращает значение, запрашивается ли справка
}

// отформатированная строка справки формируется в виде:
// <краткое_использование>
// <описание_программы>
// <позиционные аргументы>
// <опции>
// Например,
//
// Program [OPTIONS] <N...>
// Program accumulate arguments
// Positional argument:
// N,  [repeated, min args = 1]
// Options:
//      --sum,  add args[default = false]
//      --mult,  multiply args[default = false]
// -h,  --help,  Display this help and exit
//
std::string ArgParser::HelpDescription()
{
    std::ostringstream oss; // для формирования строки справки
    oss << program_name << " [OPTIONS]";

    const auto& helpOption = GetHelpOption();

    auto positionalIterator = std::find_if(options.begin(), options.end(), [](const auto& opt){
        return opt.IsPositional();
    });

    if (positionalIterator != options.end())
    {
        oss << " <" << positionalIterator->GetLongOption();
        if (positionalIterator->IsMultiValue())
            oss << "...";
        oss << '>';
    }
    oss  << '\n';

    oss << helpOption.GetDescription() << '\n';

    if (positionalIterator != options.end())
        oss << "Positional argument:\n" << *positionalIterator << '\n';

    oss << "Options:\n";
    for (const auto& opt: options)
    {
        if (opt.GetType() != OptionType::HelpOption && !opt.IsPositional())
            oss << opt << '\n';
    }
    oss << helpOption << '\n';
    return oss.str();
}

}