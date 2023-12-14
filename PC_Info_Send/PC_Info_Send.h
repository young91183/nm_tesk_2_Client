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

using json = nlohmann::json;

json getCPUInfo();
json getDiskInfo();
json getMemoryInfo();
json getNICInfo(std::string *client_ip);
std::string getCurrentDateTime();
json getInfo(std::string ip);

class Client {
public:
    Client();
    void start();
private:
    bool connectToServer();
    int server_socket;
    std::string client_ip;
};
