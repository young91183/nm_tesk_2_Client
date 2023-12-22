#include "Add_Log.h"

void add_log(std::string tag, std::string detail) {
    // 파일을 추가 모드로 연다. (ios::app)
    std::ofstream file("log_.txt", std::ios_base::app);
    std::string log = "";
    log += getCurrentDateTime() + " / #" + tag + "  / " + detail + " /\n";

    // 파일이 제대로 열렸는지 확인한다.
    if (file.is_open()) {
        // 문자열을 파일에 쓴다.
        file << log << std::endl;

        // 파일을 닫는다.
        file.close();

    } else {
        std::cerr << "파일 열기에 실패했습니다." << std::endl;
    }
}
