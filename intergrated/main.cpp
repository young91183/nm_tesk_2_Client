#include "./Account_Client/Account_Client.h"
#include "./PC_Info_Send/PC_Info_Send.h"

int main(void) {
	bool isLoop = false;
	std::string id, pw, req;
	int result = 2, log_signal = 0;
	std::thread pc_info_thread; 
	std::thread file_send_thread;
	
	while(true) {
		Account_Client *account_client = new Account_Client();
		std::cout << "[login, logout, join 중 선택]\n";
		std::cout << "Request : ";
		std::cin >> req;
		if (req == "quit") {
			isLoop = false;	
			break;
		} else if ((req == "login" or req =="join")&& log_signal == 1){
			std::cout << "이미 로그인 중입니다. 로그 아웃 후 진행 해 주세요.\n";
			continue;
		}
	//	else if(req != "logout"){
		std::cout << "ID : ";
		std::cin >> id;

		std::cout << "Password : ";
		std::cin >> pw;
	//	}

		result = account_client->start(req, id, pw);
		delete account_client;

		if(result == 1){
			isLoop = true;
			std::cout << "login에 성공하셨습니다. 시스템을 가동합니다.\n";
			log_signal = 1;
			// PC_INFO 가동
			pc_info_thread = std::thread(pc_info_start, id, pw, &isLoop);
			// 압축 전송 모듈 가동
			// 제작 중
			std::cout << "시스템 가동이 시작되었습니다.\n";

		} else if (result == 0) {
			log_signal = 0;
			std::cout << "logout 되었습니다. 시스템을 중단합니다.\n";
			// PC_INFO, 압축 전송 반복문 종료
			isLoop = false;
			// 종료
			if(pc_info_thread.joinable()) pc_info_thread.join();
			std::cout << "시스템 종료가 완료되었습니다.\n";
		} else if (result == 2) std::cout << "회원가입에 성공하였습니다.\n";
		else if (result == 3) continue;
		else { 
			std::cout << "오류로 인한 시스템 종료\n";
			return 0;
		}
	}
	std::cout << "시스템을 종료합니다.\n";
	return 0;
}
