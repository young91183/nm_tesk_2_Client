#ifndef ACCOUNT_CLIENT_H
#define ACCOUNT_CLIENT_H
#include "../PC_Info_Send/PC_Info_Send.h"
#include "../Add_Log/Add_Log.h"

using json = nlohmann::json;


class Account_Client {
public:
    Account_Client(int main_socket);
    int start(std::string req, std::string id, std::string pw);
private:
    bool connectToServer();
    int server_socket;
    std::string client_ip;
};

#endif // ACCOUNT_CLIENT_H
