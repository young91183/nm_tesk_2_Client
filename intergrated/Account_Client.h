#ifndef ACCOUNT_INFO_H
#define ACCOUNT_INFO_H
#include "PC_Info_Send.h"

using json = nlohmann::json;


class Account_Client {
public:
    Account_Client();
    int start(std::string req, std::string id, std::string pw);
private:
    bool connectToServer();
    int server_socket;
    std::string client_ip;
};

#endif // ACCOUNT_INFO_H
