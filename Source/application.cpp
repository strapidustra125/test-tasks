
/* ========================== Подключение библиотек ========================== */

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>	// Для функции sleep()
#include <cstdlib> 	// Для функций rand() и srand()
#include <ctime> 	// Для функции time()
#include <algorithm>

#include "zmq.hpp"
#include "zmq_cmd.hpp"

using namespace std;
using namespace zmq;


/* ================================ Константы ================================ */

#define ZMQ_MESSAGE_SIZE        10

// Команды Клиента
#define ZMQ_CMD_IDRequest       10
#define ZMQ_CMD_KillAll         11

// Команды сервера
#define ZMQ_CMD_ID              20



/* ========================= Вспомогательные функции ========================= */

// Новый случайный ID
int genNewID(vector<int> vecID)
{
    int newID;
    while(1)
    {
        newID = 1000 + rand() % 9000;

        if(find(vecID.begin(), vecID.end(), newID) == vecID.end())
        {
            return newID;
        }
    }

}


// Формирование строки для ZMQ сообщения
string ZMQStringFormat(int cmd, int ID)
{
    return (ID == 0) ? to_string(cmd) + "0000": to_string(cmd) + to_string(ID);
}


// Получение команды из сообщения
int getCmdFromString(string ZMQMessageString)
{
    string cmd = "";
    cmd += ZMQMessageString[0];
    cmd += ZMQMessageString[1];

    return stoi(cmd);
}


// Получение ID из сообщения
int getIDFromString(string ZMQMessageString)
{
    string cmd = "";
    cmd += ZMQMessageString[2];
    cmd += ZMQMessageString[3];
    cmd += ZMQMessageString[4];
    cmd += ZMQMessageString[5];

    return stoi(cmd);
}


// Получение передаваемой информации из ZMQ сообщения
string getStringFromZMQMessage(const zmq::message_t & aMessage)
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

    // Создание контекста
    context_t zmqContext;

    // Сокеты для запроса и ответа
    socket_t zmqRequestSocket(zmqContext, socket_type::req);
    socket_t zmqReplaySocket(zmqContext, socket_type::rep);

    // Сообщение для отправки по ZeroMQ
    message_t zmqMessage(ZMQ_MESSAGE_SIZE);

    // Команда для обмена между клиентами и сервером
    zmq_cmd zmqCommand(0, 0, 0);

    // Адрес сокета для подключения
    string socketAddress = "tcp://127.0.0.1:4444";

    // Флаг первой запущенной программы
    int serverFlag = 1;

    // ID текущего экземпляра приложения
    int currentApplicationID = 0;


    /* ======= Серверные переменные ======= */

    vector <int> IDvector;
    int newID = 0;

    string messageString = "";                //



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
        serverFlag = 0;

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

        currentApplicationID = genNewID(IDvector);
        IDvector.push_back(currentApplicationID);

        cout << "Application ID: " << currentApplicationID << endl << endl;



        while(1)
        {
            zmqReplaySocket.recv(&zmqMessage);
            messageString = getStringFromZMQMessage(zmqMessage);

            cout << "Server got: " << messageString << endl;

            switch (getCmdFromString(messageString))
            {

            /* ======= Запрос ID ======= */

            case ZMQ_CMD_IDRequest:

                cout << "Got command \"ZMQ_CMD_IDRequest\": " << ZMQ_CMD_IDRequest << endl;

                newID = genNewID(IDvector);
                messageString = ZMQStringFormat(ZMQ_CMD_ID, newID);
                IDvector.push_back(newID);

                break;


            case ZMQ_CMD_KillAll:

                break;

            default:
                break;
            }




            zmqMessage.rebuild(messageString.c_str(), ZMQ_MESSAGE_SIZE);
            zmqReplaySocket.send(zmqMessage);

            cout << "Server send: " << messageString << endl;
        }

        sleep(5);

        zmqMessage.rebuild("123456", ZMQ_MESSAGE_SIZE);
        zmqReplaySocket.send(zmqMessage);



        break;






        /* ===================== Клиентская часть ===================== */

    case false:

        cout << "It's a client!" << endl << endl;
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;

        zmqRequestSocket.connect(socketAddress);
        zmqMessage.rebuild(ZMQStringFormat(ZMQ_CMD_IDRequest, currentApplicationID).c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(zmqMessage);

        cout << "Client send: " << ZMQStringFormat(ZMQ_CMD_IDRequest, currentApplicationID) << endl;

        zmqRequestSocket.recv(&zmqMessage);
        messageString = getStringFromZMQMessage(zmqMessage);

        cout << "Client got: " << messageString << endl;

        currentApplicationID = getIDFromString(messageString);

        cout << "Application ID: " << currentApplicationID << endl << endl;


        break;

    default:
        break;
    }



    while(1)
    {

    }









    return 0;
}
