
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


std::string hexDump( const zmq::message_t & aMessage ) {
  // I'm going to build a hex/ascii dump like you've seen so many times before
  std::string  msg;
  std::string  ascii;
  // get the pointer to the start of the message's payload - and it's size
  const char *buff = (const char *)aMessage.data();
  int         size = ((zmq::message_t &)aMessage).size();
  const char *end  = buff + size - 1;
  // see if it's the trivial case
  if (buff == NULL) {
    msg.append("NULL");
  } else {
    // get a place to hold the conversion of each byte
    char   hex[3];
    bzero(hex, 3);
    // run through the valid data in the buffer
    for (const char *p = buff; p <= end; ++p) {
      // generate the hex code for the byte and add it
      snprintf(hex, 3, "%02x", (uint8_t)(*p));
      msg.append(hex).append(" ");
      // if it's printable, add it to the ascii part, otherwise, put a '.'
      if (isprint(*p)) {
        ascii.append(p, 1);
      } else {
        ascii.append(".");
      }
      // see if we have a complete line
      if (ascii.size() >= 19) {
        msg.append(" ").append(ascii).append("\n");
        ascii.clear();
      }
    }
    // if we have anything left, put it on the line as well
    if (ascii.size() > 0) {
      msg.append((19 - ascii.length())*3 + 1, ' ').append(ascii);
    }
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


    char* value = (char*)malloc(6);


    /* ======= Выбор фрагмента кода: клиент или сервер ======= */

    switch(serverFlag)
    {

        /* ===================== Серверная часть ===================== */

    case true:

        cout << "It's a server!" << endl << endl;
        cout << "Connection status: " << zmqReplaySocket.connected() << endl;

        currentApplicationID = genNewID();

        cout << "Application ID: " << currentApplicationID << endl;

        while(1)
        {
            requestMessage.rebuild();
            cout << "Message:    " << requestMessage.data() << endl;



            //zmqReplaySocket.recv(&replayMessageString, 6);
            zmqReplaySocket.recv(&replayMessage);

            cout << "Server got: " << hexDump(replayMessage) << endl;

            //sleep(1);

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
        memcpy (requestMessage.data (), ZMQStringFormat(currentApplicationID, ZMQ_CMD_IDRequest).c_str(), ZMQ_MESSAGE_SIZE);
        //requestMessage.rebuild(ZMQStringFormat(currentApplicationID, ZMQ_CMD_IDRequest).c_str(), ZMQ_MESSAGE_SIZE);
        zmqRequestSocket.send(requestMessage);

        cout << "Client send: " << ZMQStringFormat(currentApplicationID, ZMQ_CMD_IDRequest) << endl;

        zmqRequestSocket.recv(&replayMessageString, ZMQ_MESSAGE_SIZE);

        cout << "Client got: " << replayMessageString << endl;


        break;

    default:
        break;
    }



    while(1)
    {
        try
        {
            zmqRequestSocket.recv(&replayMessageString, ZMQ_MESSAGE_SIZE);
        }
        catch(zmq::error_t err)
        {

        }

        cout << "Client got: " << replayMessageString << endl;

        sleep(1);
    }




    //serverFlag = zmq_bind(zmqRepSocket, socketAddress.c_str());

    // Создание клиентского сокета
    //void * zmqReqSocket = zmq_socket(zmqContext, ZMQ_REQ);

    // context_t zmqContext;
    // socket_t zmqRepSocket(zmqContext, socket_type::push);
    // message_t msg;

    // zmqRepSocket.bind(socketAddress);

    // msg.rebuild(IDRequestString.c_str(), IDRequestString.length());

    // zmqRepSocket.send(msg);























    // if(serverFlag == 0)
    // {
    //     // Значит это - первое запущенное приложение

    //     cout << "I'm a server!" << endl;

    //     currentApplicationID = genNewID();

    //     cout << "Application ID: " << currentApplicationID << endl;
    // }
    // else
    // {
    //     // Значит сервер уже проинициализировал сокет.

    //     cout << "I'm a client!" << endl;

    //     // Подключение к открытому сервером сокету
    //     zmq_connect(zmqReqSocket, socketAddress.c_str());


    //     zmq_msg_init_size(&sendMessage, IDRequestString.length());
    //     memcpy(zmq_msg_data(&sendMessage), IDRequestString.c_str(), IDRequestString.length());
    //     rc = zmq_msg_send(&sendMessage, zmqReqSocket, 0);

    //     if(rc == -1) cout << "ID request error! rc = " << rc << "." << endl;

    //     zmq_msg_close(&sendMessage);



    //     //currentApplicationID = genNewID();

    //     //cout << "Application ID: " << currentApplicationID << endl;
    // }

    // //char * value;

    // cout << "waiting..." << endl;

    // int i = 0;

    // while(i++ < 10)
    // {
    //     //i++;
    //     zmq_msg_t receiveMessage;
    //     zmq_msg_init(&receiveMessage);

    //     rc = zmq_msg_recv(&receiveMessage, zmqRepSocket, 0);

    //     if(rc == -1)
    //     {
    //         //cout << "New message error! rc = " << rc << "." << endl;
    //         cout << rc << endl;
    //     }
    //     else
    //     {
    //         int length = zmq_msg_size(&receiveMessage);
    //         char* value = (char*)malloc(length);
    //         memcpy(value, zmq_msg_data(&receiveMessage), length);
    //         zmq_msg_close(&receiveMessage);
    //         printf("%s\n", value);
    //         free(value);

    //         // value = (char *)malloc(zmq_msg_size(&receiveMessage));
    //         // memcpy(value, zmq_msg_data(&receiveMessage), zmq_msg_size(&receiveMessage));

    //         // cout << "New message: " << value << endl;


    //     }

    //     // value = (char *)malloc(zmq_msg_size(&receiveMessage));
    //     // memcpy(value, zmq_msg_data(&receiveMessage), zmq_msg_size(&receiveMessage));

    //     // cout << "New message: " << value << endl;

    //     //zmq_msg_close(&receiveMessage);
    //     rc = 0;










    //     sleep(1);
    // }










    // cout << "Destroy context!" << endl;

    // zmq_close(zmqRepSocket);
    // zmq_close(zmqReqSocket);

    // zmq_ctx_destroy(zmqContext);

    // cout << "End of program!" << endl;



































    return 0;
}
