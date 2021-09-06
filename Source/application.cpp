
/* ========================== Подключение библиотек ========================== */

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>	// Для функции sleep()
#include <cstdlib> 	// Для функций rand() и srand()
#include <ctime> 	// Для функции time()

#include "zmq.hpp"

using namespace std;


/* ================================ Константы ================================ */

// Команды ZeroMQ
#define ZMQ_CMD_IDRequest       10


/* ========================= Вспомогательные функции ========================= */

// Строка случайных четырехзначных чисел
int genNewID()
{
    return 1000 + rand() % 9000;
}



/* ============================= Основная функция ============================ */


int main(int argc, char *argv[])
{
    cout << endl << "Starting ZMQ application." << endl << endl;

    /* ======= Переменные ======= */

    string socketAddress = "tcp://127.0.0.1:4444";  // Сокет для подключения
    int currentApplicationID = 0;                   // ID текущего экземпляра приложения
    vector<int> IDVector;                           // Вектор ID всех запущенных приложений
    int serverFlag = 0;					            // Флаг первой запущенной программы
    string IDRequestString = "Send me your ID";     // Строка запроса ID

    int rc;

    zmq_msg_t receiveMessage;
    zmq_msg_init(&receiveMessage);

    zmq_msg_t sendMessage;
    zmq_msg_init(&sendMessage);


    srand(time(0)); // Стартовое число ГПСЧ


    /* =======  ======= */



    /* ======= Конфигурация обмена сообщениями ======= */

    // Создание контекста
    void * zmqContext = zmq_ctx_new();

    // Создание серверного сокета
    void * zmqRepSocket = zmq_socket(zmqContext, ZMQ_REP);

    // Создание клиентского сокета
    void * zmqReqSocket = zmq_socket(zmqContext, ZMQ_REQ);

    // Пытаемся открыть сокет
    serverFlag = zmq_bind(zmqRepSocket, socketAddress.c_str());

    if(serverFlag == 0)
    {
        // Значит это - первое запущенное приложение

        cout << "I'm a server!" << endl;

        currentApplicationID = genNewID();

        cout << "Application ID: " << currentApplicationID << endl;
    }
    else
    {
        // Значит сервер уже проинициализировал сокет.

        cout << "I'm a client!" << endl;

        // Подключение к открытому сервером сокету
        zmq_connect(zmqReqSocket, socketAddress.c_str());


        zmq_msg_init_size(&sendMessage, IDRequestString.length());
        memcpy(zmq_msg_data(&sendMessage), IDRequestString.c_str(), IDRequestString.length());
        rc = zmq_msg_send(&sendMessage, zmqReqSocket, 0);

        if(rc == -1) cout << "ID request error! rc = " << rc << "." << endl;

        zmq_msg_close(&sendMessage);



        //currentApplicationID = genNewID();

        //cout << "Application ID: " << currentApplicationID << endl;
    }

    cout << "waiting..." << endl;

    while(1)
    {
        cout << "." << endl;

        rc = zmq_msg_recv(&receiveMessage, zmqRepSocket, ZMQ_DONTWAIT);

        if(rc == -1) cout << "New message error! rc = " << rc << "." << endl;

        char* value = (char *)malloc(zmq_msg_size(&receiveMessage));
        memcpy(value, zmq_msg_data(&receiveMessage), zmq_msg_size(&receiveMessage));

        cout << "New message: " << value << endl;

        sleep(1);
    }












    cout << "Waiting..." << endl;



    while(1)
    {
        // cout << "while..." << endl;

        // zmq_msg_recv(&receiveMessage, zmqRepSocket, 0);
        // int length = zmq_msg_size(&receiveMessage);
        // char* value = (char *)malloc(length);
        // memcpy(value, zmq_msg_data(&receiveMessage), length);
        // //zmq_msg_close(&receiveMessage);
        // printf("1:%s\n", value);
        // free(value);

        // zmq_msg_recv(&receiveMessage, zmqReqSocket, 0);


        // s = zmq_msg_gets(&receiveMessage, "Message");

        // cout << s << endl;

        // if(messageToReceive != "")
        // {
        // 	zmq_msg_close(&receiveMessage);
        // 	break;
        // }

        sleep(1); // sleep one second

        // zmq_msg_t reply;
        // zmq_msg_init_size(&reply, strlen("world"));
        // memcpy(zmq_msg_data(&reply), "world", 5);
        // zmq_msg_send(&reply, respond, 0);
        // zmq_msg_close(&reply);
    }

    // zmq_close(zmqSocket);
    // zmq_ctx_destroy(zmqContext);

    // cout << "End of program!" << endl;

    return 0;
}
