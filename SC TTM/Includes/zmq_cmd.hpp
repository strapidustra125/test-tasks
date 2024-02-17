/********************************************************************************
 *
 * Класс для хранения команды, передаваемой в сообщении ZeroMQ
 *
 *******************************************************************************/

#ifndef _ZMQ_CMD_HPP_
#define _ZMQ_CMD_HPP_

#include <string>


using namespace std;

class zmq_cmd
{
public:
    unsigned int commandID;
    unsigned int applicationID;
    unsigned int commandData;

    zmq_cmd(unsigned int cmdID=0, unsigned int appID=0, unsigned int cmdData=0)
    {
        commandID = cmdID;
        applicationID = appID;
        commandData = cmdData;
    }

    string formatString();
    void readString(string inputString);

};

string zmq_cmd::formatString()
{
    return  ((commandID == 0) ? "00" : to_string(commandID)) +
            ((applicationID == 0) ? "0000" : to_string(applicationID)) +
            ((commandData == 0) ? "0000" : to_string(commandData));
}

void zmq_cmd::readString(string inputString)
{
    string tempStr = "";
    tempStr += inputString[0];
    tempStr += inputString[1];
    commandID = stoi(tempStr);

    tempStr = "";
    tempStr += inputString[2];
    tempStr += inputString[3];
    tempStr += inputString[4];
    tempStr += inputString[5];
    applicationID = stoi(tempStr);

    tempStr = "";
    tempStr += inputString[6];
    tempStr += inputString[7];
    tempStr += inputString[8];
    tempStr += inputString[9];
    commandData = stoi(tempStr);
}































#endif
