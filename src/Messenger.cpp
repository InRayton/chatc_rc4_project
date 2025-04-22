#include "Messenger.h"
#include "RC4.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <thread>
#include <atomic>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

namespace Messenger {
    void logMessage(const string &msg) {
        cout << msg << endl; //вывод строки в консоль
    }

    int createServerSocket(int port) {
        WSADATA wsaData; //структура для хранения данных о реализации сокетов
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) { //инициализация библиотеки сокетов
            logMessage("failed"); //вывод ошибки и завершение
            exit(1);
        }

        int serverFd = socket(AF_INET, SOCK_STREAM, 0); //создание сокета
        if (serverFd < 0) {
            logMessage("socket creation error"); //ошибки создания сокета
            exit(1);
        }
        sockaddr_in serverAddr; //структура адреса сервера
        serverAddr.sin_family = AF_INET; //использование
        serverAddr.sin_port = htons(port); //установка порта с преобразованием порядка байт
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { //привязка сокета к адресу
            logMessage("socket bind error"); //обработка ошибки привязки
            exit(1);
        }

        if (listen(serverFd, 2) < 0) { //перевод сокета в режим прослушивания
            logMessage("socket listen error"); //обработка ошибки прослушивания
            exit(1);
        }
        return serverFd; //возврат дескриптора сокета
    }

    int connectToServer(const string &address, int port) {
        int clientFd = socket(AF_INET, SOCK_STREAM, 0); //создание tcp клиентского сокета
        if (clientFd < 0) {
            logMessage("client socket creation error"); //обработка ошибки
            exit(1);
        }

        sockaddr_in serverAddr; //структура адреса сервера
        serverAddr.sin_family = AF_INET; //айпиви версии 4
        serverAddr.sin_port = htons(port); //порт сервера
        serverAddr.sin_addr.S_un.S_addr = inet_addr(address.c_str()); //айпи-адрес сервера

        if (connect(clientFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) { //установка соединения
            logMessage("server connection error"); //обработка ошибки подключения
            exit(1);
        }

        return clientFd; //возврат декриптора клиента
    }

    void sendMessage(int socketFd, const string &message, const string &key) {
        RC4 cipher(key); //создание объекта шифрования RC4
        string encrypted = cipher.crypt(message); //шифрование сообщения
        uint32_t len = static_cast<uint32_t>(encrypted.size()); //получение длины зашифрованного сообщения
        len = htonl(len); //преобразование порядка байт
        send(socketFd, reinterpret_cast<char*>(&len), sizeof(len), 0); //отправка длины сообщения
        send(socketFd, encrypted.data(), encrypted.size(), 0); //отправка самого зашифрованного сообщения
    }

    string receiveMessage(int socketFd, const string &key) {
        uint32_t len = 0; //переменная для длины входящего сообщения
        int received = recv(socketFd, reinterpret_cast<char*>(&len), sizeof(len), 0); //получение длины сообщения
        if (received <= 0)
            return ""; //возврат пустой строки при ошибке или завершении соединения
        len = ntohl(len); //преобразование порядка байт
        string encrypted(len, '\0'); //выделение нужной длины
        size_t total = 0; //счётчик принятых байт
        while (total < len) { //чтение сообщения по частям
            received = recv(socketFd, &encrypted[total], len - total, 0); //приём данных
            if (received <= 0)
                break; //выход при ошибке
            total += received; //обновление счётчика
        }
        RC4 cipher(key); //создание объекта RC4
        string decrypted = cipher.crypt(encrypted); //расшифровка данных
        return decrypted; //возврат расшифрованного сообщения
    }

    void startServer() {
        const int port = 12345; //порт сервера
        int serverFd = createServerSocket(port); //создание серверного сокета
        logMessage("server started. waiting for users..."); //вывод статуса

        sockaddr_in clientAddr1, clientAddr2; //структуры адресов клиентов
        int addrLen = sizeof(clientAddr1); //длина структуры

        int clientFd1 = accept(serverFd, (sockaddr*)&clientAddr1, &addrLen); //ожидание первого клиента
        logMessage("first client connected!"); //логирование подключения
        int clientFd2 = accept(serverFd, (sockaddr*)&clientAddr2, &addrLen); //ожидание второго клиента
        logMessage("second client connected!"); //логирование подключения

        atomic<bool> running(true); //флаг работы
        thread t1([&]() {
            while (running.load()) { //цикл при активном соединении
                string msg = receiveMessage(clientFd1, "secret"); //получение сообщения от клиента 1
                if (msg.empty())
                    break; //выход при ошибке
                logMessage("from client 1: " + msg); //логирование сообщения
                sendMessage(clientFd2, msg, "secret"); //отправка клиенту 2
            }
        });
        thread t2([&]() {
            while (running.load()) { //цикл для второго клиента
                string msg = receiveMessage(clientFd2, "secret"); //получение сообщения от клиента 2
                if (msg.empty())
                    break;
                logMessage("from client 2: " + msg); //логирование
                sendMessage(clientFd1, msg, "secret"); //отправка клиенту 1
            }
        });

        t1.join(); //ожидание завершения потока
        t2.join(); //ожидание завершения второго потока

        closesocket(clientFd1); //закрытие соединений
        closesocket(clientFd2);
        closesocket(serverFd); //закрытие серверного сокета
        WSACleanup();
        logMessage("server stopped"); //логирование завершения работы
    }

    void startClient() {
        const string serverAddress = "127.0.0.1"; //айпи адрес сервера
        const int port = 12345; //порт сервера
        int clientFd = connectToServer(serverAddress, port); //подключение к серверу
        logMessage("Connected to server " + serverAddress + ":" + to_string(port)); //лог

        string key = "secret"; //ключ шифрования
        atomic<bool> running(true); //флаг работы

        thread receiveThread([&]() { //создание потока приёма сообщений
            while (running.load()) {
                string msg = receiveMessage(clientFd, key); //приём сообщения
                if (msg.empty())
                    break; //выход при отключении
                logMessage("Received: " + msg); //логирование
            }
        });

        while (running.load()) { //основной цикл ввода
            string input;
            getline(cin, input); //ввод с консоли
            if (input == "/exit") { //команда выхода
                running.store(false); //выход из цикла
                break;
            }
            sendMessage(clientFd, input, key); //отправка сообщения
        }

        receiveThread.join(); //ожидание завершения потока
        closesocket(clientFd); //закрытие сокета
        WSACleanup(); //очистка ресурсов
        logMessage("client stopped"); //логирование завершения
    }

}
