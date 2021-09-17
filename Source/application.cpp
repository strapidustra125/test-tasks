
/* ========================== Подключение библиотек ========================== */

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>	// Для функции sleep()
#include <cstdlib> 	// Для функций rand() и srand()
#include <ctime> 	// Для функции time()
#include <algorithm>

#include "../Includes/command.pb.h"
#include "../Includes/zmq_cmd.hpp"

#include "zmq.hpp"


using namespace std;
using namespace zmq;


/* ================================ Константы ================================ */

#define DETAIL_LOG
#define DETAIL_EXCHANGE_LOG
#define DETAIL_CONNECT_LOG

#define ZMQ_MESSAGE_SIZE        10

#define ZMQ_SERVER              1
#define ZMQ_CLIENT              2

// Команды Клиента
#define ZMQ_CMD_IDRequest       10
#define ZMQ_CMD_moreID          11
#define ZMQ_CMD_KillAll         12

// Команды сервера
#define ZMQ_CMD_ID              20
#define ZMQ_CMD_IDVector        21


/* ================================ Переменные ================================ */

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
    int serverFlag = 0;

    // ID текущего экземпляра приложения
    int currentApplicationID = 0;

    // ID сервера
    int serverID = 0;

    // Вектор ID запущенных приложений
    vector <int> IDvector;



    string s;

    ZMQ_cmd ZMQCommand;



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


// Обнуление значения переменных для нового сервера
void initNewServer()
{
    // Удаляем ID сервера из вектора
    vector<int>::iterator serverIDIter = find(IDvector.begin(), IDvector.end(), serverID);
    if(serverIDIter != IDvector.end()) IDvector.erase(serverIDIter);

    serverFlag = ZMQ_SERVER;

    serverID = currentApplicationID;
}




/* ============================= Основная функция ============================ */


int main(/*int argc, char *argv[]*/)
{
    cout << endl << "Starting ZMQ application..." << endl << endl;

    srand(time(0)); // Стартовое число ГПСЧ



    /* ======= Конфигурация обмена сообщениями ======= */

    // Определение роли приложения: клиент или сервер
    try
    {
        // Пытаемся открыть сокет

        #ifdef DETAIL_CONNECT_LOG
            cout << "Try to bind a socket..." << endl;
        #endif // DETAIL_CONNECT_LOG

        zmqReplaySocket.bind(socketAddress);

        #ifdef DETAIL_CONNECT_LOG
            cout << "Success!" << endl;
        #endif // DETAIL_CONNECT_LOG

        serverFlag = ZMQ_SERVER;
    }
    catch(zmq::error_t err)
    {
        // Если не получилось, значит сервер его уже открыл

        #ifdef DETAIL_CONNECT_LOG
            cout << "Extension: \"" << err.what() << "\"." << endl;
        #endif // DETAIL_CONNECT_LOG

        serverFlag = ZMQ_CLIENT;

        zmqRequestSocket.connect(socketAddress);

        #ifdef DETAIL_CONNECT_LOG
            cout << "Connection status: " <<
            (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
            << " with \"" << socketAddress << "\"" << endl;
            //cout << "Close socket..." << endl << endl;
        #endif // DETAIL_CONNECT_LOG

        //zmqReplaySocket.close();


    }


/* ================== Рабочий цикл приложения ================== */

    while(1)
    {

        /* ======= Выбор фрагмента кода: клиент или сервер ======= */

        switch(serverFlag)
        {

    /* ===================== Серверная часть ===================== */

        case ZMQ_SERVER:

            #ifdef DETAIL_LOG
                cout << endl << "====>  It's a server!" << endl << endl;
                cout << "Connection status: " <<
                    (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
                    << " with \"" << socketAddress << "\"" << endl << endl;
            #endif // DETAIL_LOG

            if(IDvector.empty())
            {
                currentApplicationID = genNewID(IDvector);
                serverID = currentApplicationID;
                IDvector.push_back(serverID);
            }

            cout << "Application ID: " << currentApplicationID << endl;

            printIDVector(IDvector);

            while(1)
            {
                zmqReplaySocket.recv(&zmqMessage);

                ZMQCommand.ParseFromArray(zmqMessage.data(), zmqMessage.size());

                // s = getStringFromZMQMessage(zmqMessage);

                // cout << "\tProtobuf: |" << s << "|" << endl;

                // ZMQCommand.ParseFromString(s);

                #ifdef DETAIL_EXCHANGE_LOG

                    cout << "\t\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\t\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\t\tapplicationID: " << ZMQCommand.applicationid() << endl;
                #endif // DETAIL_EXCHANGE_LOG


                //ZMQCommand.SerializePartialToArray();

                zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

                switch (zmqCommand.commandID)
                {


            /* ======= Запрос ID ======= */

                case ZMQ_CMD_IDRequest:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_IDRequest\": " << ZMQ_CMD_IDRequest << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    IDvector.push_back(genNewID(IDvector));

                    zmqCommand.commandID = ZMQ_CMD_ID;
                    zmqCommand.applicationID = currentApplicationID;
                    zmqCommand.commandData = IDvector.back();

                    break;


            /* ======= Запрос следующего ID вектора ======= */

                case ZMQ_CMD_moreID:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_moreID\": " << ZMQ_CMD_moreID << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    if(zmqCommand.commandData < IDvector.size())
                    {
                        zmqCommand.applicationID = IDvector[zmqCommand.commandData];
                        zmqCommand.commandData = currentApplicationID;
                    }
                    else zmqCommand.applicationID = 0;

                    // Потому что для приема и передачи один экземпляр класса zmq_cmd
                    swap(zmqCommand.applicationID, zmqCommand.commandData);

                    zmqCommand.commandID = ZMQ_CMD_IDVector;

                    break;


            /* ======= Выключить все приложения ======= */

                case ZMQ_CMD_KillAll:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_KillAll\": " << ZMQ_CMD_KillAll << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    break;

                default:
                    break;
                }

                zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
                zmqReplaySocket.send(zmqMessage);

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << "Server send message: " << zmqCommand.formatString() << endl;
                #endif // DETAIL_EXCHANGE_LOG
            }

            break;




    /* ===================== Клиентская часть ===================== */

        case ZMQ_CLIENT:

            #ifdef DETAIL_LOG
                cout << endl << "====>  It's a client!" << endl << endl;
            #endif // DETAIL_LOG

        /* ======= Запрос ID ======= */

            // Запрос ID у сервера

            ZMQCommand.set_applicationid(currentApplicationID);
            ZMQCommand.set_commandid(ZMQ_CMD_IDRequest);
            ZMQCommand.set_commanddata(0);

            #ifdef DETAIL_EXCHANGE_LOG
                cout << "\t\tcommandID: " << ZMQCommand.commandid() << endl;
                cout << "\t\tcommandData: " << ZMQCommand.commanddata() << endl;
                cout << "\t\tapplicationID: " << ZMQCommand.applicationid() << endl;
            #endif // DETAIL_EXCHANGE_LOG

            s = "123";
            //if(!ZMQCommand.SerializeAsString(&s)) cout << "ERROR" << endl;

            s = ZMQCommand.SerializePartialAsString();

            cout << "-" << endl;
            for(int i = 0; i < s.length(); i++) cout << s[i] << endl;
            cout << "-" << endl;

            cout << "Protobuf: |" << s << "|" << endl;

            zmqMessage.rebuild(s.c_str(), ZMQCommand.ByteSizeLong());
            zmqRequestSocket.send(zmqMessage);





            // zmqCommand.commandID = ZMQ_CMD_IDRequest;
            // zmqCommand.applicationID = currentApplicationID;

            // zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            // zmqRequestSocket.send(zmqMessage);

            // #ifdef DETAIL_EXCHANGE_LOG
            //     cout << "Client send message: " << zmqCommand.formatString() << endl;
            // #endif // DETAIL_EXCHANGE_LOG


            // Получение ID от сервера


            zmqRequestSocket.recv(&zmqMessage);

            ZMQCommand.ParseFromString(getStringFromZMQMessage(zmqMessage));

            #ifdef DETAIL_EXCHANGE_LOG
                cout << "\tProtobuf: " << endl;
                cout << "\t\tcommandID: " << ZMQCommand.commandid() << endl;
                cout << "\t\tcommandData: " << ZMQCommand.commanddata() << endl;
                cout << "\t\tapplicationID: " << ZMQCommand.applicationid() << endl;
            #endif // DETAIL_EXCHANGE_LOG

            currentApplicationID = ZMQCommand.commanddata();

            serverID = ZMQCommand.applicationid();



            // zmqRequestSocket.recv(&zmqMessage);

            // zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

            // currentApplicationID = zmqCommand.commandData;
            // serverID = zmqCommand.applicationID;

            cout << "Application ID: " << currentApplicationID << endl;

            #ifdef DETAIL_LOG
                cout << "Server ID: " << serverID << endl;
            #endif // DETAIL_LOG




        /* ======= Запрос вектора ID активных приложений ======= */

            // Запрос очередного ID у сервера

            zmqCommand.commandID = ZMQ_CMD_moreID;
            zmqCommand.applicationID = currentApplicationID;
            zmqCommand.commandData = IDvector.size();

            zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqRequestSocket.send(zmqMessage);

            #ifdef DETAIL_EXCHANGE_LOG
                cout << "Client send message: " << zmqCommand.formatString() << endl;
            #endif // DETAIL_EXCHANGE_LOG

            while(1)
            {
                // Получение очередного ID от сервера

                zmqRequestSocket.recv(&zmqMessage);
                zmqCommand.readString(getStringFromZMQMessage(zmqMessage));

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << "Client got message: " << zmqCommand.formatString() << endl;
                    cout << "New ID: " << zmqCommand.commandData << endl;
                #endif // DETAIL_EXCHANGE_LOG

                if(zmqCommand.commandData == 0) break;
                else IDvector.push_back(zmqCommand.commandData);


                // Запрос очередного ID у сервера

                zmqCommand.commandID = ZMQ_CMD_moreID;
                zmqCommand.applicationID = currentApplicationID;
                zmqCommand.commandData = IDvector.size();

                zmqMessage.rebuild(zmqCommand.formatString().c_str(), ZMQ_MESSAGE_SIZE);
                zmqRequestSocket.send(zmqMessage);

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << "Client send message: " << zmqCommand.formatString() << endl;
                #endif // DETAIL_EXCHANGE_LOG
            }

            printIDVector(IDvector);



        /* ======= Рабочий цикл клиента ======= */

            while (1)
            {
                #ifdef DETAIL_LOG
                    cout << "---------------------- New client iter ---------------------" << endl << endl;
                #endif // DETAIL_LOG

                try
                {
                    #ifdef DETAIL_CONNECT_LOG
                        cout << "Try to rebind a socket..." << endl;
                    #endif // DETAIL_CONNECT_LOG

                    zmqReplaySocket.bind(socketAddress);

                    #ifdef DETAIL_CONNECT_LOG
                        cout << "Success..." << "  Server was turned off!" << endl;
                        cout << "Init a new server..." << endl;
                    #endif // DETAIL_CONNECT_LOG

                    initNewServer();

                    break;
                }
                catch(zmq::error_t err)
                {
                    #ifdef DETAIL_CONNECT_LOG
                        cout << "Extension: \"" << err.what() << "\"." << endl;
                        cout << "Server is active!" << endl;
                    #endif // DETAIL_CONNECT_LOG
                }





                //zmqReplaySocket.close();
                //zmqReplaySocket.unbind(socketAddress);

                sleep(2);
            }








            break;

        default:

            cout << "====> Error: Unknown device type!" << endl;

            return 0;

            break;
        }
    }



    return 0;
}
