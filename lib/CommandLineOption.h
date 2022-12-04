#pragma once

#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <functional>
#include <stdexcept>

namespace ArgumentParser
{

// Класс перечисления типа аргумента
enum class OptionType
{
    FlagOption,     // булевый аргумент (флаг)
    IntegerOption,  // целочисленный
    StringOption,   // строковой
    HelpOption      // опция справки (помощь)
};

// Класс описывает одну опцию или аргумент командной строки
class CommandLineOption
{
    // псевдонимы для удобства

    // обобщенный тип для хранения ссылки на (внешний для объекта опции) объект
    template<typename T>
    using Ref = std::reference_wrapper<T>;

    // обобщенный тип вектора
    template<typename T>
    using Vec = std::vector<T>;

    // Тип значения опции/аргумента, переданной программе при ее вызове.
    // Возможные типы: флаг(bool), целое(int) или строка(string), либо monostate, если объект еще не содержит значения.
    using ValueType = std::variant<std::monostate, bool, int, std::string>;

    // Тип массива значений опции/аргумента для случая MultiValue
    using ArrayType = std::variant<Vec<bool>, Vec<int>, Vec<std::string>>;

    // Тип хранимого в объекте CommandLineOption значения опции/аргумента.
    // В зависимости является ли объект MultiValue, хранит значение или массив значений.
    using ArgumentStorageType = std::variant<ValueType, ArrayType>;

    // Тип значения для хранения ссылки на внешний объект, куда сохраняется значение опции объекта.
    using ValueRefType = std::variant<Ref<bool>, Ref<int>, Ref<std::string>>;

    // Тип значения для хранения ссылки на внешний массив объектов, куда сохраняются значения опции объекта (MultiValue).
    using ArrayRefType = std::variant<Ref<Vec<bool>>, Ref<Vec<int>>, Ref<Vec<std::string>>>;

    // Тип хранимого в объекте CommandLineOption ссылки на внешнее хранилище значений опции/аргумента.
    // В зависимости является ли объект MultiValue, хранит ссылку на объект или массив объектов (MultiValue),
    // либо monostate, если внешнее хранилище не указано.
    using ExternalStorageType = std::variant<std::monostate, ValueRefType, ArrayRefType>;

public:
    // Конструктор принимает тип опции, краткую опцию, длинную опцию и описание
    CommandLineOption(OptionType optionType, char shortOpt, std::string longOpt, std::string desc);

    // Установить значение по умолчанию для флага
    CommandLineOption& Default(bool value);

    // Установить значение по умолчанию для целого
    CommandLineOption& Default(int value);

    // Установить значение по умолчанию для строки
    CommandLineOption& Default(std::string value);
    // перегрузка для устранения неопределенности с bool версией.
    CommandLineOption& Default(const char* value) { return Default(std::string{value}); }

    // Установить, что опция будет иметь несколько значений (минимум minArgsCount)
    CommandLineOption& MultiValue(size_t minArgsCount = 0);

    // Установить, что текущий объект - позиционный аргумент
    CommandLineOption& Positional();

    // Указать внешний объект для сохранения значения опции
    CommandLineOption& StoreValue(bool& ref);

    // Указать внешний объект для сохранения значения опции/аргумента
    CommandLineOption& StoreValue(int& ref);

    // Указать внешний объект для сохранения значения опции/аргумента
    CommandLineOption& StoreValue(std::string& ref);

    // Указать внешний объект для сохранения массива значений опции/аргумента
    CommandLineOption& StoreValues(std::vector<int>& ref);

    // Указать внешний объект для сохранения массива значений опции/аргумента
    CommandLineOption& StoreValues(std::vector<std::string>& ref);

    // То же, что и выше, для массивов
    // В тесте PositionalArgTest используется данный метод вместо StoreValues
    // Если это опечатка в тесте, то данные два метода ниже можно удалить (после исправления опечатки)
    CommandLineOption& StoreValue(std::vector<int>& ref) { return StoreValues(ref); }
    CommandLineOption& StoreValue(std::vector<std::string>& ref) { return StoreValues(ref); }

    // Определено ли значение по умолчанию для данной опции
    bool HasDefault() const;

    // Получить значение по умолчанию для флага
    bool GetDefaultFlag() const;

    // Получить значение по умолчанию для целого
    int GetDefaultInt() const;

    // Получить значение по умолчанию для строки
    const std::string& GetDefaultString() const;

    // Получить значение флага
    bool GetFlag() const;

    // Получить значение целого
    int GetInt() const;

    // Получить значение целого из массива значений в позиции pos (MultiValue)
    int GetInt(size_t pos) const;

    // Получить значение строки
    const std::string& GetString() const;

    // Получить значение строки из массива значений в позиции pos (MultiValue)
    const std::string& GetString(size_t pos) const;

    // Установить значение флага
    CommandLineOption& SetValue(bool value);

    // Установить или добавить (для MultiValue) значение целого
    CommandLineOption& SetValue(int value);

    // Установить или добавить (для MultiValue) значение строки
    CommandLineOption& SetValue(const std::string& value);

    // Позиционный ли аргумент
    bool IsPositional() const { return is_positional; }

    // Хранит ли множество значений
    bool IsMultiValue() const { return is_multi_value; }

    // Минимальное количество значений (для MultiValue)
    size_t GetMinArgs() const { return min_args_count; }

    // Тип данной опции (флаг/целое/строка)
    OptionType GetType() const { return option_type; }

    // Короткая опция
    char GetShortOption() const { return short_opt; }

    // Длинная опция
    const std::string& GetLongOption() const { return long_opt; }

    // Описание опции
    const std::string& GetDescription() const { return description; }

    // Проверка на корректность объекта опции
    bool IsValid() const;

private:
    OptionType option_type;                 // Тип данной опции
    const char short_opt;                   // Короткая опция
    const std::string long_opt;             // Длинная опция
    const std::string description;          // Описание
    ValueType default_value;                // Значение по умолчанию
    ArgumentStorageType argument_values;    // Хранимое значение (значение или массив значений для MultiValue)
    ExternalStorageType external_values;    // Ссылка на внешнее значение (значение или массив значений для MultiValue)
    bool is_positional = false;             // Позиционный ли аргумент
    bool is_multi_value = false;            // Хранит ли множество значений (MultiValue)
    size_t min_args_count = 0;              // Минимальное количество значений (для MultiValue)
};

// Оператор вывода опции в поток. Выводит информацию о ней:
// короткое, длинное имя, описание, значение по умолчанию (если есть), повторяемое (MultiValue) ли и сколько раз минимум.
std::ostream& operator<<(std::ostream& os, const CommandLineOption& opt);

}
