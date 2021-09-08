
/* ========================== Подключение библиотек ========================== */

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>	// Для функции sleep()
#include <cstdlib> 	// Для функций rand() и srand()
#include <ctime> 	// Для функции time()

#include "zmq.hpp"

using namespace std;
using namespace zmq;


/* ================================ Константы ================================ */

#define ZMQ_MESSAGE_SIZE        6

// Команды ZeroMQ
#define ZMQ_CMD_IDRequest       10
#define ZMQ_CMD_KillAll         11


/* ========================= Вспомогательные функции ========================= */

// Строка случайных четырехзначных чисел
int genNewID()
{
    return 1000 + rand() % 9000;
}


// Формирование строки для ZMQ сообщения
string ZMQStringFormat(int ID, int cmd)
{
    return (ID == 0) ? "0000" + to_string(cmd): to_string(ID) + to_string(cmd);
}


string hexDump(const zmq::message_t & aMessage)
{
    // I'm going to build a hex/ascii dump like you've seen so many times before
    string  msg;
    string  ascii;

    const char *buff = (const char *)aMessage.data();       // Указатели на начало сообщения
    int         size = ((zmq::message_t &)aMessage).size(); // Размер сообщения
    const char *end  = buff + size - 1;                     // Указатель на конец сообщения

    if (buff == NULL) msg.append("NULL");
    else
    {
        // run through the valid data in the buffer
        for (const char *p = buff; p <= end; ++p)
        {
            if (isprint(*p)) ascii.append(p, 1);
            else ascii.append(".");

            // see if we have a complete line
            if (ascii.size() >= 19)
            {
                msg.append(ascii).append("\n");
                ascii.clear();
            }
        }

        // if we have anything left, put it on the line as well
        if (ascii.size() > 0) msg.append(ascii);
    }

    return msg;
}


/* ============================= Основная функция ============================ */


int main(int argc, char *argv[])
{
    cout << endl << "Starting ZMQ application." << endl << endl;

    /* ======= Общие переменные ======= */

    context_t zmqContext;                           // Создание контекста
    socket_t zmqReplaySocket(zmqContext, socket_type::rep);
    socket_t zmqRequestSocket(zmqContext, socket_type::req);

    string socketAddress = "tcp://127.0.0.1:4444";  // Сокет для подключения
    bool serverFlag = true;					        // Флаг первой запущенной программы
    int currentApplicationID = 0;                   // ID текущего экземпляра приложения

    message_t   requestMessage(ZMQ_MESSAGE_SIZE),                     // ZMQ сообщение для запроса
                replayMessage(ZMQ_MESSAGE_SIZE);                      // ZMQ сообщение для ответа

    string replayMessageString = "";                //


    /* ======= Серверные переменные ======= */

    vector <int> IDvector;


    /* ======= Клиентские переменные ======= */




    srand(time(0)); // Стартовое число ГПСЧ


    /* ======= Конфигурация обмена сообщениями ======= */

    // Определение роли приложения: клиент или сервер
    try
    {
        // Пытаемся открыть сокет
        zmqReplaySocket.bind(socketAddress);
    }
    catch(zmq::error_t err)
    {
        // Если не получилось, значит сервер его уже открыл

        cout << "Extension..." << endl;
        serverFlag = false;

        if(zmqReplaySocket.connected())
        {
            cout << "Connection status: " << zmqReplaySocket.connected() << endl;
            cout << "It's connected to \"" << socketAddress << "\"." << endl;
            zmqReplaySocket.close();
            cout << "Close socket..." << endl;
        }
    }


    /* ======= Выбор фрагмента кода: клиент или сервер ======= */

    switch(serverFlag)
    {

        /* ===================== Серверная часть ===================== */

    case true:

        cout << "It's a server!" << endl << endl;
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;

        currentApplicationID = genNewID();

        cout << "Application ID: " << currentApplicationID << endl << endl;

        while(1)
        {
            zmqReplaySocket.recv(&replayMessage);

            cout << "Server got: " << hexDump(replayMessage) << endl;

            replayMessage.rebuild("123456", 6);
            zmqReplaySocket.send(replayMessage);

            cout << "Server send: 123456" << endl;
        }


        break;






        /* ===================== Клиентская часть ===================== */

    case false:

        cout << "It's a client!" << endl << endl;
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;
        replayMessageString = "";

        zmqRequestSocket.connect(socketAddress);
        requestMessage.rebuild(ZMQStringFormat(currentApplicationID, ZMQ_CMD_IDRequest).c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(requestMessage);

        cout << "Client send: " << ZMQStringFormat(currentApplicationID, ZMQ_CMD_IDRequest) << endl;

        zmqRequestSocket.recv(&replayMessage);

        cout << "Client got: " << hexDump(replayMessage) << endl;


        break;

    default:
        break;
    }



    while(1)
    {

    }









    return 0;
}
