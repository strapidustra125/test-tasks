
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

#define DETAIL_LOG

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
        //cout << ".";
    }

    cout << endl;

    //cout << ".";
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

        #ifdef DETAIL_LOG
            cout << "Extension..." << endl;
        #endif // DETAIL_LOG

        serverFlag = ZMQ_CLIENT;

        if(zmqReplaySocket.connected())
        {

            #ifdef DETAIL_LOG
                cout << "Connection status: " << zmqReplaySocket.connected() << endl;
                cout << "It's connected to \"" << socketAddress << "\"." << endl;
                cout << "Close socket..." << endl << endl;
            #endif // DETAIL_LOG

            zmqReplaySocket.close();

        }
    }







    /* ======= Выбор фрагмента кода: клиент или сервер ======= */

    switch(serverFlag)
    {

/* ===================== Серверная часть ===================== */

    case ZMQ_SERVER:

        #ifdef DETAIL_LOG
            cout << "It's a server!" << endl << endl;
            cout << "Connection status: " <<
                (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
                << " with \"" << socketAddress << "\"" << endl << endl;
        #endif // DETAIL_LOG

        currentApplicationID = genNewID(IDvector);
        IDvector.push_back(currentApplicationID);


        cout << "Application ID: " << currentApplicationID << endl;

        printIDVector(IDvector);

        while(1)
        {
            zmqReplaySocket.recv(&zmqMessage);
            zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

            switch (zmqCommand.commandID)
            {


        /* ======= Запрос ID ======= */

            case ZMQ_CMD_IDRequest:

                #ifdef DETAIL_LOG
                    cout << "Got command \"ZMQ_CMD_IDRequest\": " << ZMQ_CMD_IDRequest << endl;
                #endif // DETAIL_LOG

                IDvector.push_back(genNewID(IDvector));

                zmqCommand.commandID = ZMQ_CMD_ID;
                zmqCommand.applicationID = IDvector.back();

                break;


        /* ======= Запрос следующего ID вектора ======= */

            case ZMQ_CMD_moreID:

                #ifdef DETAIL_LOG
                    cout << "Got command \"ZMQ_CMD_moreID\": " << ZMQ_CMD_moreID << endl;
                #endif // DETAIL_LOG

                if(zmqCommand.commandData < IDvector.size())
                {
                    zmqCommand.applicationID = IDvector[zmqCommand.commandData];
                }
                else zmqCommand.applicationID = 0;

                zmqCommand.commandID = ZMQ_CMD_IDVector;

                break;


        /* ======= Выключить все приложения ======= */

            case ZMQ_CMD_KillAll:

                #ifdef DETAIL_LOG
                    cout << "Got command \"ZMQ_CMD_KillAll\": " << ZMQ_CMD_KillAll << endl;
                #endif // DETAIL_LOG

                break;

            default:
                break;
            }

            zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqReplaySocket.send(zmqMessage);

            #ifdef DETAIL_LOG
                cout << "Server send message: " << zmqCommand.formatString() << endl;
            #endif // DETAIL_LOG
        }

        break;




/* ===================== Клиентская часть ===================== */

    case ZMQ_CLIENT:

        #ifdef DETAIL_LOG
            cout << "It's a client!" << endl << endl;
            cout << "Connection status: " <<
                (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
                << " with \"" << socketAddress << "\"" << endl << endl;
            cout << "Trying to connect..." << endl;
        #endif // DETAIL_LOG

        zmqRequestSocket.connect(socketAddress);

        #ifdef DETAIL_LOG
            cout << "New connection status: " <<
                (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
                << " with \"" << socketAddress << "\"" << endl << endl;
        #endif // DETAIL_LOG



    /* ======= Запрос ID ======= */

        // Запрос ID у сервера

        zmqCommand.commandID = ZMQ_CMD_IDRequest;
        zmqCommand.applicationID = currentApplicationID;

        zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(zmqMessage);

        #ifdef DETAIL_LOG
            cout << "Client send message: " << zmqCommand.formatString() << endl;
        #endif // DETAIL_LOG


        // Получение ID от сервера

        zmqRequestSocket.recv(&zmqMessage);
        zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

        currentApplicationID = zmqCommand.applicationID;

        cout << "Application ID: " << currentApplicationID << endl;



    /* ======= Запрос вектора ID активных приложений ======= */

        // Запрос очередного ID у сервера

        zmqCommand.commandID = ZMQ_CMD_moreID;
        zmqCommand.applicationID = currentApplicationID;
        zmqCommand.commandData = IDvector.size();

        zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(zmqMessage);

        #ifdef DETAIL_LOG
            cout << "Client send message: " << zmqCommand.formatString() << endl;
        #endif // DETAIL_LOG

        while(1)
        {
            // Получение очередного ID от сервера

            zmqRequestSocket.recv(&zmqMessage);
            zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

            #ifdef DETAIL_LOG
                cout << "Client got message: " << zmqCommand.formatString() << endl;
                cout << "New ID: " << zmqCommand.applicationID << endl;
            #endif // DETAIL_LOG

            if(zmqCommand.applicationID == 0) break;
            else IDvector.push_back(zmqCommand.applicationID);


            // Запрос очередного ID у сервера

            zmqCommand.commandID = ZMQ_CMD_moreID;
            zmqCommand.applicationID = currentApplicationID;
            zmqCommand.commandData = IDvector.size();

            zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqRequestSocket.send(zmqMessage);

            #ifdef DETAIL_LOG
                cout << "Client send message: " << zmqCommand.formatString() << endl;
            #endif // DETAIL_LOG
        }

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
