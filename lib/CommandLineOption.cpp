#include "CommandLineOption.h"

#include <ostream>

namespace ArgumentParser
{

CommandLineOption::CommandLineOption(OptionType optionType, char shortOpt, std::string longOpt, std::string desc)
        : option_type(optionType)
        , short_opt(shortOpt)
        , long_opt(std::move(longOpt))
        , description(std::move(desc))
{
    if (option_type == OptionType::HelpOption)
        argument_values = false; // опция справки - специальный тип флага: всегда false, если не задать специально (запросить справку)
    if (option_type == OptionType::FlagOption)
        default_value = false; // для флагов по умолчанию false
}

CommandLineOption& CommandLineOption::Default(bool value)
{
    if (option_type != OptionType::FlagOption) // опция должна быть флагом
        throw std::logic_error("Option is not a Flag");
    default_value = value; // устанавливаем значение по умолчанию
    return *this;
}

CommandLineOption& CommandLineOption::Default(int value)
{
    if (option_type != OptionType::IntegerOption) // опция должна быть целым числом
        throw std::logic_error("Option is not an Integer");
    default_value = value; // устанавливаем значение по умолчанию
    return *this;
}

CommandLineOption& CommandLineOption::Default(std::string value)
{
    if (option_type != OptionType::StringOption) // опция должна быть строкой
        throw std::logic_error("Option is not a String");
    default_value.emplace<std::string>(std::move(value)); // устанавливаем значение по умолчанию
    return *this;
}

CommandLineOption& CommandLineOption::MultiValue(size_t minArgsCount)
{
    is_multi_value = true;
    min_args_count = minArgsCount;
    // MultiValue имеет значение для чисел и строк, флаги не могут быть MultiValue
    if (option_type == OptionType::IntegerOption)
        argument_values.emplace<ArrayType>(Vec<int>{});
    else if (option_type == OptionType::StringOption)
        argument_values.emplace<ArrayType>(Vec<std::string>{});
    else
        throw std::logic_error("Option can not be MultiValue");
    return *this;
}

CommandLineOption& CommandLineOption::Positional()
{
    // Позиционными аргументами могут быть только числа и строки
    if (option_type != OptionType::IntegerOption && option_type != OptionType::StringOption)
        throw std::logic_error("Option can not be Positional");
    is_positional = true;
    return *this;
}

CommandLineOption& CommandLineOption::StoreValue(bool& ref)
{
    if (option_type != OptionType::FlagOption)
        throw std::logic_error("Option is not a Flag");
    external_values.emplace<ValueRefType>(ref); // сохраняем ссылку на внешний объект для записи значения
    return *this;
}

CommandLineOption& CommandLineOption::StoreValue(int& ref)
{
    if (option_type != OptionType::IntegerOption)
        throw std::logic_error("Option is not an Integer");
    external_values.emplace<ValueRefType>(ref); // сохраняем ссылку на внешний объект для записи значения
    return *this;
}

CommandLineOption& CommandLineOption::StoreValue(std::string& ref)
{
    if (option_type != OptionType::StringOption)
        throw std::logic_error("Option is not a String");
    external_values.emplace<ValueRefType>(ref); // сохраняем ссылку на внешний объект для записи значения
    return *this;
}

CommandLineOption& CommandLineOption::StoreValues(std::vector<int>& ref)
{
    if (option_type != OptionType::IntegerOption)
        throw std::logic_error("Option is not an Integer");
    external_values.emplace<ArrayRefType>(ref); // сохраняем ссылку на внешний объект-массив для записи значений
    return *this;
}

CommandLineOption& CommandLineOption::StoreValues(std::vector<std::string>& ref)
{
    if (option_type != OptionType::StringOption)
        throw std::logic_error("Option is not a String");
    external_values.emplace<ArrayRefType>(ref); // сохраняем ссылку на внешний объект-массив для записи значений
    return *this;
}

bool CommandLineOption::HasDefault() const
{
    return default_value.index() != 0; // нулевой индекс при monostate (нет значения по умолчанию)
}

bool CommandLineOption::GetDefaultFlag() const
{
    return std::get<bool>(default_value); // получаем и возвращаем булево значение (значение флага) по умолчанию
}

int CommandLineOption::GetDefaultInt() const
{
    return std::get<int>(default_value); // получаем и возвращаем целое по умолчанию
}

const std::string& CommandLineOption::GetDefaultString() const
{
    return std::get<std::string>(default_value); // получаем и возвращаем строку по умолчанию
}

bool CommandLineOption::GetFlag() const
{
    if (std::get<ValueType>(argument_values).index() == 0) // если значения нет (monostate)
        return GetDefaultFlag(); // возвращаем булево значение (значение флага) по умолчанию
    return std::get<bool>(std::get<ValueType>(argument_values)); // иначе, возвращаем сохраненное значение флага
}

int CommandLineOption::GetInt() const
{
    if (std::get<ValueType>(argument_values).index() == 0) // значения нет
        return GetDefaultInt(); // возвращаем значение по умолчанию
    return std::get<int>(std::get<ValueType>(argument_values)); // иначе, возвращаем значение
}

int CommandLineOption::GetInt(size_t pos) const
{
    // возвращаем значение числа в позиции pos массива сохраненных значений (MultiValue)
    return std::get<Vec<int>>(std::get<ArrayType>(argument_values)).at(pos);
}

const std::string& CommandLineOption::GetString() const
{
    if (std::get<ValueType>(argument_values).index() == 0) // значения нет
        return GetDefaultString(); // возвращаем значение по умолчанию
    return std::get<std::string>(std::get<ValueType>(argument_values)); // иначе, возвращаем значение
}

const std::string& CommandLineOption::GetString(size_t pos) const
{
    // возвращаем значение строки в позиции pos массива сохраненных значений (MultiValue)
    return std::get<Vec<std::string>>(std::get<ArrayType>(argument_values)).at(pos);
}

CommandLineOption& CommandLineOption::SetValue(bool value)
{
    if (option_type != OptionType::FlagOption && option_type != OptionType::HelpOption)
        throw std::logic_error("Option is not a Flag or Help");

    argument_values = value; // устанавливаем значение флага
    if (external_values.index() != 0) // если есть ссылка на внешнее хранилище (индекс хранимого типа не monostate)
        std::get<Ref<bool>>(std::get<ValueRefType>(external_values)).get() = value; // записываем значение в это хранилище
    return *this;
}

CommandLineOption& CommandLineOption::SetValue(int value)
{
    if (option_type != OptionType::IntegerOption)
        throw std::logic_error("Option is not an Integer");

    if (argument_values.index() == 0) // если храним одиночное значение (не MultiValue)
        argument_values = value; // устанавливаем его
    else // иначе (MultiValue), добавляем значение в массив
        std::get<Vec<int>>(std::get<ArrayType>(argument_values)).push_back(value);

    if (external_values.index() != 0) // если есть ссылка на внешнее хранилище (индекс хранимого типа не monostate)
    {
        if (is_multi_value) // При MultiValue, хранимая ссылка на внешнее хранилище - ссылка на массив. Добавляем в него значение
            std::get<Ref<Vec<int>>>(std::get<ArrayRefType>(external_values)).get().push_back(value);
        else // иначе, записываем значение по хранимой ссылке на внешнее хранилище
            std::get<Ref<int>>(std::get<ValueRefType>(external_values)).get() = value;
    }
    return *this;
}

CommandLineOption& CommandLineOption::SetValue(const std::string& value)
{
    if (option_type != OptionType::StringOption)
        throw std::logic_error("Option is not a String");

    if (argument_values.index() == 0)
        argument_values =value;
    else
        std::get<Vec<std::string>>(std::get<ArrayType>(argument_values)).push_back(value);

    if (external_values.index() != 0)
    {
        if (is_multi_value)
            std::get<Ref<Vec<std::string>>>(std::get<ArrayRefType>(external_values)).get().push_back(value);
        else
            std::get<Ref<std::string>>(std::get<ValueRefType>(external_values)).get() = value;
    }
    return *this;
}

bool CommandLineOption::IsValid() const
{
    if (!is_multi_value && std::get<ValueType>(argument_values).index() == 0) // если не MultiValue, и нет значения (monostate)
        return HasDefault(); // возвращаем, есть ли значение по умолчанию для данной опции

    if (is_multi_value) // если MultiValue
    {
        size_t count = 0; // количество сохраненных значений
        if (option_type == OptionType::IntegerOption) // если храним числа
            count = std::get<Vec<int>>(std::get<ArrayType>(argument_values)).size();
        else if (option_type == OptionType::StringOption) // если строки
            count = std::get<Vec<std::string>>(std::get<ArrayType>(argument_values)).size();
        if (count < min_args_count) // если количество сохраненных значений меньше минимального
            return false; // объект не корректен (нет/недостаточно обязательных значений)
    }
    return true;
}

// формирование и вывод в поток отформатированной строки с информацией об опции в виде:
// -<короткое_имя>,  --<длинное_имя>,  <описание> [<по_умолчанию> | <повторы>]
// для позиционного аргумента <короткое_имя> не выводится, тире отсутствуют.
std::ostream& operator<<(std::ostream& os, const CommandLineOption& opt)
{
    if (!opt.IsPositional()) // не позиционный аргумент
    {
        if (opt.GetShortOption())
            os << '-' << opt.GetShortOption() << ",  "; // выводим короткое имя
        else
            os << "     ";
        os << "--";
    }
    os << opt.GetLongOption() << ",  "; // длинное имя

    OptionType optionType = opt.GetType();
    if (optionType == OptionType::HelpOption) // в случае с опцией справки
    {
        os << "Display this help and exit"; // имеем специальное описание
    }
    else
    {
        os << opt.GetDescription(); // выводим описание
        if (opt.IsMultiValue()) // повтор и минимальное количество раз (для MultiValue)
            os << "[repeated, min args = " << opt.GetMinArgs() << "]";
        else if (opt.HasDefault()) // если есть значение по умолчанию
        { // выводим его
            os << "[default = ";
            if (optionType == OptionType::FlagOption)
                os << std::boolalpha << opt.GetDefaultFlag();
            else if (optionType == OptionType::IntegerOption)
                os << opt.GetDefaultInt();
            else
                os << opt.GetDefaultString();
            os << "]";
        }
    }
    return os;
}

}