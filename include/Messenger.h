#ifndef MESSENGER_H
#define MESSENGER_H
#include <string>
#include <thread>
#include <atomic>
using namespace std;

namespace Messenger { //пространство имён для функций сокетов и шифрования
    //функция запуска сервера
    void startServer();   // запуск сервера
    //функция запуска клиента
    void startClient();
    //функция отправки сообщения
    void sendMessage(int socketFd, const string &message, const string &key);
    //функция получения сообщения
    string receiveMessage(int socketFd, const string &key);
    //функция создания серверного сокета
    int createServerSocket(int port);
    //функция подключения к серверу
    int connectToServer(const string &address, int port);
    //функция логирования сообщений
    void logMessage(const string &msg);
}

#endif
