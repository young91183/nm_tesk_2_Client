#include "Account_Client.h"


// Client 통신 구현 부
Account_Client::Account_Client() : server_socket(socket(AF_INET, SOCK_STREAM, 0)) { 
    if (server_socket < 0) {
        std::perror("socket");
        exit(1);
    }
}


int Account_Client::start(std::string req, std::string id, std::string pw) {
	nlohmann::json account_data, j;
	std::string request_message, message, res_massage;
	char buffer[500];
	j = getNICInfo(&client_ip);
	if (!connectToServer()) exit(1);
	request_message = "account";
	request_message += '\0';
	
	ssize_t n = write(server_socket, request_message.c_str(), request_message.size());
	if (n <= 0) {
		if (n < 0) {
			std::cout << "write err\n";
			return -1;
		}
	}

	account_data["request"] = req;
	account_data["id"] = id;
	account_data["password"] = pw;
	account_data["up_time"] = getCurrentDateTime();
	account_data["ip"] = client_ip;

	message = account_data.dump();
	message += '\0';
	n = write(server_socket, message.c_str(), message.size());

	if (n <= 0) {
		if (n < 0) {
			std::cout << "write err\n";
			return -1;
		}
	}

	n = read(server_socket, buffer, sizeof(buffer)-1);

	if (n < 0){ 
		std::perror("read");
		std::cout << "Client read error\n";
		return -1;
	} else {
		close(server_socket);
		res_massage = buffer;
		if(res_massage == "l_s") return 1;
		else if(res_massage == "lo_s") return 0;
		else if(res_massage == "j_s") return 2;
		else {
			std::cout << res_massage << std::endl;
			return 3;
		};
	}
}

bool Account_Client::connectToServer() {
	sockaddr_in server_address{};
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		std::perror("connect_account");
		return false; // Connect 실패 시 false 반환
	}
	return true;
}
