#include "dynamic_lib.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <stdexcept>

// Функция 1. Сортирует элементы входной строки по убыванию и все четные элементы заменяет на латинские буквы «КВ».
// Функция не возвращает никакого значения.
void function1(std::string& str) {
    // Сортировка по убыванию
    std::sort(str.begin(), str.end(), std::greater<char>());

    std::string result = "";
    for (char c : str) {
        if (std::isdigit(c)) {
            int val = c - '0';
            if (val % 2 == 0) {
                result += "KB";
            } else {
                result += c;
            }
        } else {
            result += c;
        }
    }
    str = result;
}

// Функция 2. Рассчитывает и возвращает общую сумму всех элементов входной строки, которые являются численными значениями.
int function2(const std::string& str) {
    int sum = 0;
    for (char c : str) {
        if (std::isdigit(c)) {
            sum += (c - '0');
        }
    }
    return sum;
}

// Функция 3. Анализирует, из скольки символов состоит входная строка.
// Если оно больше 2-ух символов и, если оно кратно 32, то функция возвращает «истина»
// (это относится к значению суммы, а не к длине строки). В противном случае функция возвращает «ложь».
bool function3(const std::string& str) {
    try {
        if (str.empty()) return false;
        
        int sum_val = std::stoi(str);
        
        // По примечанию "(это относится к значению суммы, а не к длине строки)"
        // Проверяем, что само значение суммы больше 2 и кратно 32.
        return (sum_val > 2 && sum_val % 32 == 0);
    } catch (...) {
        return false;
    }
}
