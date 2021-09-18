
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
//#define DETAIL_EXCHANGE_LOG
#define DETAIL_CONNECT_LOG

#define ZMQ_MESSAGE_SIZE        10

#define ZMQ_SERVER              1
#define ZMQ_CLIENT              2

// Команды Клиента
#define ZMQ_CMD_IDRequest       10
#define ZMQ_CMD_moreID          11
#define ZMQ_CMD_KillAll         12
#define ZMQ_CMD_KillCheck       13

// Команды сервера
#define ZMQ_CMD_ID              20
#define ZMQ_CMD_IDVector        21
#define ZMQ_CMD_Kill            22


/* ================================ Переменные ================================ */

    // Создание контекста
    context_t zmqContext;

    // Сокеты для запроса и ответа
    socket_t zmqRequestSocket(zmqContext, socket_type::req);
    socket_t zmqReplaySocket(zmqContext, socket_type::rep);

    // Сообщение для отправки по ZeroMQ
    message_t zmqMessage(ZMQ_MESSAGE_SIZE);

    // Команда для обмена между клиентами и сервером
    ZMQ_cmd ZMQCommand;

    // Адрес сокета для подключения
    string socketAddress = "tcp://127.0.0.1:4444";



    // Флаг первой запущенной программы
    int flag_isServer = 0;

    // ID текущего экземпляра приложения
    int currentApplicationID = 0;

    // ID сервера
    int serverID = 0;

    // Вектор ID запущенных приложений
    vector <int> IDvector;

    // Флаг выключения всех команд
    bool flag_killAll = false;







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

//
void printIDVector(vector<int> vecID)
{
    cout << endl << "Actieve applications ID:" << endl;

    for(vector<int>::iterator it = vecID.begin(); it < vecID.end(); it++)
    {
        cout << "\t" << *it << endl;
    }

    cout << endl;
}

// Обнуление значения переменных для нового сервера
void initNewServer()
{
    // Удаляем ID сервера из вектора
    vector<int>::iterator serverIDIter = find(IDvector.begin(), IDvector.end(), serverID);
    if(serverIDIter != IDvector.end()) IDvector.erase(serverIDIter);

    flag_isServer = ZMQ_SERVER;

    serverID = currentApplicationID;
}


void checkArguments(int argNumber, char *argList[])
{
    #ifdef DETAIL_LOG
        cout << endl << "Application parametrs number: " << argNumber << endl;
        cout << endl << "Application parametrs list: " << endl;

        for(int i = 0; i < argNumber; i++)
        {
            cout << "\t" << i << ": " << argList[i] << endl;
        }

        cout << endl;
    #endif // DETAIL_LOG

    // Чтобы гарантировать корректность посимвольного сравнения
    string key(argList[1]);

    // Если указан только один ключ
    if(argNumber <= 2)
    {
        // Проверка значения ключа
        if((key == "-k") || (key == "--kill"))
        {
            // Закрыть все активные приложения

            cout << "kill" << endl;

            flag_killAll = true;
        }
        else if(key == "-h" || key == "--help")
        {
            // Вывести справку

            cout << "help" << endl;

            exit(0);
        }
        else
        {
            cout << endl <<  "Error: Invalid parametr value!" << endl;
            cout << "Exit." << endl << endl;

            exit(0);
        }
        

        

    }
    else
    {
        cout << endl << "Error: Too much params!" << endl;
        cout << "Exit." << endl << endl;

        exit(0);
    }
}


/* ============================= Основная функция ============================ */


int main(int argc, char *argv[])
{
    cout << endl << "Starting ZMQ application..." << endl << endl;

    srand(time(0)); // Стартовое число ГПСЧ

    int temp;

    checkArguments(argc, argv);

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

        flag_isServer = ZMQ_SERVER;
    }
    catch(zmq::error_t err)
    {
        // Если не получилось, значит сервер его уже открыл

        #ifdef DETAIL_CONNECT_LOG
            cout << "Extension: \"" << err.what() << "\"." << endl;
        #endif // DETAIL_CONNECT_LOG

        flag_isServer = ZMQ_CLIENT;

        zmqRequestSocket.connect(socketAddress);

        #ifdef DETAIL_CONNECT_LOG
            cout << "Connection status: " <<
            (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
            << " with \"" << socketAddress << "\"" << endl;
            //cout << "Close socket..." << endl << endl;
        #endif // DETAIL_CONNECT_LOG
    }


/* ================== Рабочий цикл приложения ================== */

    while(1)
    {

        /* ======= Выбор фрагмента кода: клиент или сервер ======= */

        switch(flag_isServer)
        {

    /* ===================== Серверная часть ===================== */

        case ZMQ_SERVER:

            #ifdef DETAIL_LOG
                cout << endl << "====>  It's a server!" << endl << endl;
                cout << "Connection status: " <<
                    (zmqReplaySocket.connected() == 1 ? "connected" : "disconnected")
                    << " with \"" << socketAddress << "\"" << endl << endl;
            #endif // DETAIL_LOG

            // Если первое приложение запущено с ключом выключения всех
            if(flag_killAll)
            {
                cout << endl << "\"Kill All\" command was found." << endl;
                cout << "Closing application." << endl << endl;

                exit(0);
            }

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

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf recieved struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
                #endif // DETAIL_EXCHANGE_LOG

                switch (ZMQCommand.commandid())
                {


            /* ======= Запрос ID ======= */

                case ZMQ_CMD_IDRequest:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_IDRequest\": " << ZMQ_CMD_IDRequest << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    IDvector.push_back(genNewID(IDvector));

                    #ifdef DETAIL_LOG
                        cout << endl << "New ID: " << IDvector.back() << endl << endl;
                    #endif // DETAIL_LOG

                    ZMQCommand.set_commandid(ZMQ_CMD_ID);
                    ZMQCommand.set_applicationid(currentApplicationID);
                    ZMQCommand.set_commanddata(IDvector.back());

                    break;


            /* ======= Запрос следующего ID вектора ======= */

                case ZMQ_CMD_moreID:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_moreID\": " << ZMQ_CMD_moreID << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    if(ZMQCommand.commanddata() < IDvector.size())
                    {
                        ZMQCommand.set_applicationid(IDvector[ZMQCommand.commanddata()]);
                        ZMQCommand.set_commanddata(currentApplicationID);
                    }
                    else ZMQCommand.set_applicationid(0);

                    // Потому что для приема и передачи один экземпляр класса zmq_cmd
                    temp = ZMQCommand.commanddata();
                    ZMQCommand.set_commanddata(ZMQCommand.applicationid());
                    ZMQCommand.set_applicationid(temp);

                    ZMQCommand.set_commandid(ZMQ_CMD_IDVector);

                    break;


            /* ======= Выключить все приложения ======= */

                case ZMQ_CMD_KillAll:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_KillAll\": " << ZMQ_CMD_KillAll << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    break;


            /* ======= Проверка необходимости выключения ======= */

                case ZMQ_CMD_KillCheck:

                    #ifdef DETAIL_EXCHANGE_LOG
                        cout << "Got command \"ZMQ_CMD_KillCheck\": " << ZMQ_CMD_KillCheck << endl;
                    #endif // DETAIL_EXCHANGE_LOG

                    if(1)
                    {
                        ZMQCommand.set_commandid(ZMQ_CMD_Kill);
                        ZMQCommand.set_applicationid(currentApplicationID);
                        ZMQCommand.set_commanddata(0);
                    }
                    else
                    {

                    }


                    break;


                default:
                    break;
                }

                zmqMessage.rebuild(ZMQCommand.SerializeAsString().c_str(), ZMQ_MESSAGE_SIZE);
                zmqReplaySocket.send(zmqMessage);

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf replied struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
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
                cout << endl << "Protobuf replied struct:" << endl;
                cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
            #endif // DETAIL_EXCHANGE_LOG

            zmqMessage.rebuild(ZMQCommand.SerializeAsString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqRequestSocket.send(zmqMessage);





            // Получение ID от сервера

            zmqRequestSocket.recv(&zmqMessage);

            ZMQCommand.ParseFromArray(zmqMessage.data(), zmqMessage.size());

            #ifdef DETAIL_EXCHANGE_LOG
                cout << endl << "Protobuf recieved struct:" << endl;
                cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
            #endif // DETAIL_EXCHANGE_LOG

            currentApplicationID = ZMQCommand.commanddata();

            serverID = ZMQCommand.applicationid();

            cout << "Application ID: " << currentApplicationID << endl;

            #ifdef DETAIL_LOG
                cout << "Server ID: " << serverID << endl;
            #endif // DETAIL_LOG




        /* ======= Запрос вектора ID активных приложений ======= */

            // Запрос очередного ID у сервера

            ZMQCommand.set_commandid(ZMQ_CMD_moreID);
            ZMQCommand.set_applicationid(currentApplicationID);
            ZMQCommand.set_commanddata(IDvector.size());

            zmqMessage.rebuild(ZMQCommand.SerializeAsString().c_str(), ZMQ_MESSAGE_SIZE);
            zmqRequestSocket.send(zmqMessage);

            #ifdef DETAIL_EXCHANGE_LOG
                cout << endl << "Protobuf replied struct:" << endl;
                cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
            #endif // DETAIL_EXCHANGE_LOG

            while(1)
            {
                // Получение очередного ID от сервера

                zmqRequestSocket.recv(&zmqMessage);
                ZMQCommand.ParseFromArray(zmqMessage.data(), zmqMessage.size());

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf recieved struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
                #endif // DETAIL_EXCHANGE_LOG

                if(ZMQCommand.commanddata() == 0) break;
                else IDvector.push_back(ZMQCommand.commanddata());


                // Запрос очередного ID у сервера

                ZMQCommand.set_commandid(ZMQ_CMD_moreID);
                ZMQCommand.set_applicationid(currentApplicationID);
                ZMQCommand.set_commanddata(IDvector.size());

                zmqMessage.rebuild(ZMQCommand.SerializeAsString().c_str(), ZMQ_MESSAGE_SIZE);
                zmqRequestSocket.send(zmqMessage);

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf replied struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
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



            /* ======= Отключение по ключу '-k' ======= */

                // Запрос

                ZMQCommand.set_commandid(ZMQ_CMD_KillCheck);
                ZMQCommand.set_applicationid(currentApplicationID);
                ZMQCommand.set_commanddata(0);

                zmqMessage.rebuild(ZMQCommand.SerializeAsString().c_str(), ZMQ_MESSAGE_SIZE);
                zmqRequestSocket.send(zmqMessage);

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf replied struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
                #endif // DETAIL_EXCHANGE_LOG


                // Ответ

                zmqRequestSocket.recv(&zmqMessage);
                ZMQCommand.ParseFromArray(zmqMessage.data(), zmqMessage.size());

                #ifdef DETAIL_EXCHANGE_LOG
                    cout << endl << "Protobuf recieved struct:" << endl;
                    cout << "\tcommandID: " << ZMQCommand.commandid() << endl;
                    cout << "\tcommandData: " << ZMQCommand.commanddata() << endl;
                    cout << "\tapplicationID: " << ZMQCommand.applicationid() << endl << endl;
                #endif // DETAIL_EXCHANGE_LOG

                if(ZMQCommand.commanddata() == 1)
                {
                    // Пришла команда выключения

                    cout << endl << "\"Kill All\" command was found." << endl;
                    cout << "Closing application." << endl << endl;
                }




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
