#include "PC_Info_Send.h"

int main() {
	int pc_num = atoi(PC_NUM); // PC_NUM을 정수형으로 변환해 사용
    std::string id, pw;
    std::cout << "ID : ";
    std::cin >> id;
    std::cout << "Password : ";
    std::cin >> pw;

	while(true) {
		auto now = std::chrono::system_clock::now(); // 현재시간 추출
		// 현재시간을 초단위로 변경해 계산이 가능하도록 변환
		auto now_in_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
		if (now_in_sec % 3 == pc_num % 3){ // 고유번호를 이용해 Client의 첫 요청을 분산
			Client().start(id, pw); 
			break; 
		} else {
        	// 10억분의 1초마다 반복문 재시작
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	}

	return 0;
}
