#include "./Account_Client/Account_Client.h"
#include "./PC_Info_Send/PC_Info_Send.h"
#include "./File_Send/File_Send.h"
#include "./Add_Log/Add_Log.h"

int main(void) {
	bool isLoop = false;
	std::string id, pw, req, account_msg, res_msg;
	int result = 2, log_signal = 0;
	std::thread pc_info_thread; 
	std::thread file_send_thread;
	int server_socket, file_send_socket;
	while(true) {
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (server_socket < 0) {
			std::perror("socket");
			exit(1);
    	}
		add_log("Service", "on");
		Account_Client *account_client = new Account_Client(server_socket);
		std::cout << "[login, logout, join 중 선택]\n";
		std::cout << "[시스템 종료 : quit]\n";
		std::cout << "Request : ";
		std::cin >> req;
		if (req == "quit") {
			isLoop = false;	
			break;
		} else if ((req == "login" || req =="join")&& log_signal == 1){
			std::cout << "이미 로그인 중입니다. 로그 아웃 후 진행 해 주세요.\n";
			add_log("account", "already login err");
			continue;
		}

		// 로그인 상태에서 logout 요청이 아닐때만 id, pw 입력받기
		if(!(req == "logout" && log_signal== 1)){ 
			std::cout << "ID : ";
			std::cin >> id;
			std::cout << "Password : ";
			std::cin >> pw;
		}

		account_msg = req + "_" + id;
		add_log("Account", account_msg);
		
		result = account_client->start(req, id, pw);
		delete account_client;
		
		res_msg = id;
		// 로그인 응답
		if(result == 1){
			isLoop = true;
			res_msg += " login success!";
			add_log("Account", res_msg);
			std::cout << "login에 성공하셨습니다. 시스템을 가동합니다.\n";
			log_signal = 1;
			// PC_INFO 가동
			pc_info_thread = std::thread(pc_info_start, id, pw, &isLoop);
			std::cout << "PC상태 정보전송이 시작되었습니다.\n";
			// 압축 전송 모듈 가동
			file_send_socket = server_socket;
			file_send_thread = std::thread(make_file, file_send_socket, &isLoop);
			std::cout << "서버의 압축파일 요청 대기를 시작했습니다.\n";

		// 로그아웃 응답
		} else if (result == 0) {
			res_msg += " logout success!";
			add_log("Account", res_msg);
			std::cout << "logout 되었습니다. ";
			close(server_socket);
			if(log_signal== 1) close(file_send_socket);
			// PC_INFO, 압축 전송 반복문 종료
			log_signal = 0;
			isLoop = false;
			// 종료
			if(pc_info_thread.joinable()) pc_info_thread.join();
			if(file_send_thread.joinable()) file_send_thread.join();
			std::cout << "작업을 중단했습니다.\n";
		
		// 회원 가입 응답
		} else if (result == 2) {
			res_msg += " join success!";
			add_log("Account", res_msg);
			std::cout << "회원가입에 성공하였습니다.\n";
			close(server_socket);	
		}

		// 계정처리 중 오류 발생 시
		else if (result == 3) {
			close(server_socket);
			continue;
		}

		// 기타 예외 발생 시
		else {
			res_msg = "account_err";
			add_log("Account", res_msg);
			std::cout << "알 수 없는 오류로 인한 시스템 종료\n";
			close(server_socket);
			break;
			return 0;
		}
	}
	add_log("Service", "off");
	std::cout << "시스템을 종료합니다.\n";
	return 0;
}
