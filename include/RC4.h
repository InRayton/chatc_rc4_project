#ifndef RC4_H
#define RC4_H
#include <string>
#include <vector>
using namespace std;

class RC4 { //класс RC4 для шифрования/расшифровки
public:
    RC4(const string &key); //конструкт принимающий ключ
    //функция шифрования/расшифровки
    string crypt(const string &data);
private:
    void init(const string &key); //инициализации состояния шифра с использованием ключа
    vector<unsigned char> s; //массив s для алгоритма RC4
    int i, j;//индексы для состояния RC4
};

#endif
