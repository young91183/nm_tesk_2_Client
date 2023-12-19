#include "Account_Client.h"


// Client 통신 구현 부
Account_Client::Account_Client() : server_socket(socket(AF_INET, SOCK_STREAM, 0)) { 
    if (server_socket < 0) {
        std::perror("socket");
        exit(1);
    }
}

// 요청정보, id, password를 받아서 계정정보 동작하는 부분
int Account_Client::start(std::string req, std::string id, std::string pw) {
	nlohmann::json account_data, j;
	std::string request_message, message, res_massage;
	char buffer[500];
	j = getNICInfo(&client_ip);
	if (!connectToServer()) exit(1);

	// Server의 Server에 account 동작임을 알림
	request_message = "account"; 
	request_message += '\0';
	ssize_t n = write(server_socket, request_message.c_str(), request_message.size());
	if (n <= 0) {
		if (n < 0) {
			std::cout << "write err\n";
			return -1;
		}
	}


	// Account_Server 모듈에 전송할 json 파일 생성 후 전송
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

	// 계정 정보 + 요청에 대한 결과 반환 받기
	n = read(server_socket, buffer, sizeof(buffer)-1);
	if (n < 0){ 
		std::perror("read");
		std::cout << "Client read error\n";
		return -1;
	} else { // 제대로 반환이 되었을 경우
		close(server_socket); 
		res_massage = buffer;
		if(res_massage == "l_s") return 1; // 로그인 완료일 경우
		else if(res_massage == "lo_s") return 0; // 로그아웃 완료 일 경우
		else if(res_massage == "j_s") return 2; // 회원가입 완료 일 경우
		else { // 기타 오류 메세지인 경우
			std::cout << res_massage << std::endl;
			return 3; 
		};
	}
}

bool Account_Client::connectToServer() { // Connect 후 특이사항 반환하는 부분
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
