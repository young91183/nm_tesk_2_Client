#ifndef PC_INFO_SEND_H
#define PC_INFO_SEND_H

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <cstring>
#include <iostream> 
#include <nlohmann/json.hpp> 
#include <chrono> 
#include <thread> 
#include <fstream> 
#include <string> 
#include <sstream> 
#include <sys/statvfs.h> 

#define PC_NUM "2"
#define SERVER_IP "10.0.2.163"
#define SERVER_PORT 8081

using json = nlohmann::json;

json getCPUInfo();
json getDiskInfo();
json getMemoryInfo();
json getNICInfo(std::string *client_ip);
std::string getCurrentDateTime();
nlohmann::json getInfo(std::string ip, std::string id, std::string pw);
void pc_info_start(std::string id, std::string pw, bool * isLoop);

class PC_Info_Client {
public:
    PC_Info_Client();
    int start(std::string id, std::string pw, bool* isLoop);
private:
    bool connectToServer();
    int server_socket;
    std::string client_ip;
};

#endif // PC_INFO_SEND_H
