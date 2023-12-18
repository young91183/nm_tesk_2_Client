#include "Account_Client.h"


// Client 통신 구현 부
Account_Client::Account_Client() : server_socket(socket(AF_INET, SOCK_STREAM, 0)) { 
    if (server_socket < 0) {
        std::perror("socket");
        exit(1);
    }
}

void Account_Client::start(std::string req, std::string id, std::string pw) {
	nlohmann::json account_data, j;
	std::string total_buffer = "";
	char buffer[500];
	j = getNICInfo(&client_ip);
	if (!connectToServer()) exit(1);

	account_data["request"] = req;
	account_data["id"] = id;
	account_data["password"] = pw;
	account_data["up_time"] = getCurrentDateTime();
	account_data["ip"] = client_ip;

	std::string message = account_data.dump();
	message += '\0';
	ssize_t n = write(server_socket, message.c_str(), message.size());

	if (n <= 0) {
		if (n < 0) std::cout << "write err\n";
		exit(1);
	}

	n = read(server_socket, buffer, sizeof(buffer)-1);

	if (n < 0){ 
		std::perror("read");
		std::cout << "Client read error\n";
		exit(1);
	} else {
		std::cout << buffer << std::endl;
	}
	close(server_socket);
}

bool Account_Client::connectToServer() {
	sockaddr_in server_address{};
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8081);
	server_address.sin_addr.s_addr = inet_addr("192.168.0.16");

	if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		std::perror("connect");
		return false; // Connect 실패 시 false 반환
	}
	return true;
}
