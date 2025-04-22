#include "RC4.h"
#include <algorithm>
using namespace std;

RC4::RC4(const string &key) : s(256), i(0), j(0) { //инициализация состояния
    init(key);//инициализация шифра с ключом
}

void RC4::init(const string &key) { //инициализация состояния RC4 с использованием ключа
    for (int idx = 0; idx < 256; ++idx) { //цикл от 0 до 255
        s[idx] = static_cast<unsigned char>(idx);//присвоение значения элементу массива s
    }
    j = 0; //установка в 0
    int keyLength = key.size(); //получение длины ключа
    for (int idx = 0; idx < 256; ++idx) { //цикл для алгоритма планирования ключа
        j = (j + s[idx] + static_cast<unsigned char>(key[idx % keyLength])) % 256; //обновление с использованием ключа и s
        swap(s[idx], s[j]); //обмен элементов массива
    }
    i = 0; //сброс i в 0
    j = 0; //сброс j в 0
}

string RC4::crypt(const string &data) { //функция для шифрования/расшифровки данных
    string result; //строка для результата
    result.resize(data.size()); //изменение размера результата в соответствии с размером входных данных
    for (size_t idx = 0; idx < data.size(); ++idx) { //цикл по каждому байту данных
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        swap(s[i], s[j]); //обмен элементов массива s
        unsigned char rnd = s[(s[i] + s[j]) % 256]; //генерация случайного байта из массива s
        result[idx] = data[idx] ^ rnd; //применение xor для шифрования/расшифровки
    }
    return result; //возврат результата
}
