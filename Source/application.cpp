
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

#define ZMQ_CLIENT              0
#define ZMQ_SERVER              1

// Команды Клиента
#define ZMQ_CMD_IDRequest       10
#define ZMQ_CMD_moreID          11
#define ZMQ_CMD_KillAll         12

// Команды сервера
#define ZMQ_CMD_ID              20
#define ZMQ_CMD_IDVector        21



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

void printIDVector(vector<int> vecID)
{
    cout << endl << "Actieve applications ID:" << endl;

    for(vector<int>::iterator it = vecID.begin(); it < vecID.end(); it++)
    {
        cout << "\t" << *it << endl;
    }

    cout << endl;
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


    /* ============== Переменные ============== */

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
    int serverFlag = ZMQ_SERVER;

    // ID текущего экземпляра приложения
    int currentApplicationID = 0;

    // Вектор ID запущенных приложений
    vector <int> IDvector;






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
        serverFlag = ZMQ_CLIENT;

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

    case ZMQ_SERVER:

        cout << "It's a server!" << endl << endl;
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;

        currentApplicationID = genNewID(IDvector);
        IDvector.push_back(currentApplicationID);
        printIDVector(IDvector);

        cout << "Application ID: " << currentApplicationID << endl << endl;



        while(1)
        {
            zmqReplaySocket.recv(&zmqMessage);
            zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

            cout << "Server got: " << zmqCommand.formatString() << endl;

            switch (zmqCommand.commandID)
            {

            /* ======= Запрос ID ======= */

            case ZMQ_CMD_IDRequest:

                cout << "Got command \"ZMQ_CMD_IDRequest\": " << ZMQ_CMD_IDRequest << endl;

                IDvector.push_back(genNewID(IDvector));

                zmqCommand.commandID = ZMQ_CMD_ID;
                zmqCommand.applicationID = IDvector.back();

                break;



            case ZMQ_CMD_moreID:

                cout << "Got command \"ZMQ_CMD_moreID\": " << ZMQ_CMD_moreID << endl;

                if(zmqCommand.commandData < IDvector.size())
                {
                    zmqCommand.applicationID = IDvector[zmqCommand.commandData];
                }
                else zmqCommand.applicationID = 0;

                zmqCommand.commandID = ZMQ_CMD_IDVector;


                break;


            case ZMQ_CMD_KillAll:

                break;

            default:
                break;
            }




            zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqReplaySocket.send(zmqMessage);

            cout << "Server send: " << zmqCommand.formatString() << endl;
        }



        break;






        /* ===================== Клиентская часть ===================== */

    case ZMQ_CLIENT:

        cout << "It's a client!" << endl << endl;
        cout << "Start connection status: " << zmqReplaySocket.connected() << endl;

        zmqRequestSocket.connect(socketAddress);
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;



        // Запрос ID
        zmqCommand.commandID = ZMQ_CMD_IDRequest;
        zmqCommand.applicationID = currentApplicationID;

        zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(zmqMessage);

        cout << "Client send: " << zmqCommand.formatString() << endl;






        zmqRequestSocket.recv(&zmqMessage);
        zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

        currentApplicationID = zmqCommand.applicationID;




        // Запрос вектора ID
        zmqCommand.commandID = ZMQ_CMD_moreID;
        zmqCommand.applicationID = currentApplicationID;
        zmqCommand.commandData = IDvector.size();

        zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(zmqMessage);

        cout << "Client send: " << zmqCommand.formatString() << endl;






        while(1)
        {
            zmqRequestSocket.recv(&zmqMessage);
            zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

            cout << "Client got: " << zmqCommand.formatString() << endl;

            if(zmqCommand.applicationID == 0) break;
            else IDvector.push_back(zmqCommand.applicationID);



            // Запрос вектора ID
            zmqCommand.commandID = ZMQ_CMD_moreID;
            zmqCommand.applicationID = currentApplicationID;
            zmqCommand.commandData = IDvector.size();

            zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqRequestSocket.send(zmqMessage);

            cout << "Client send: " << zmqCommand.formatString() << endl;

        }

        cout << "Application ID: " << currentApplicationID << endl << endl;

        printIDVector(IDvector);








        break;

    default:
        break;
    }



    while(1)
    {

    }









    return 0;
}
