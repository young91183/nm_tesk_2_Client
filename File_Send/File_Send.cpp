#include "File_Send.h"

#define BUFFER_SIZE 512

// 파일을 압축하는 함수
int compress_file(const std::string &file_path) {
    // 입력 파일을 이진 모드로 열기
    std::ifstream ifs(file_path.c_str(), std::ios_base::binary);
    // 출력 파일 (압축 파일)을 이진 모드로 생성
    std::ofstream ofs("resp_server.lz4", std::ios_base::binary);
    int i = 0;
    // 버퍼 선언
    char buffer[BUFFER_SIZE];
    // 파일 끝에 도달할 때까지 읽기를 반복
    while (ifs.read(buffer, sizeof(buffer))) {
        // 오류때문에 10번이상 반복하면 에러 return
        if (i > 10) return -2; 

        // 읽은 바이트 수를 가져오기
        size_t read_size = ifs.gcount();

        // 압축에 필요한 최대 출력 크기 계산
        size_t max_dst_size = LZ4F_compressFrameBound(read_size, NULL);

        // 압축된 데이터를 저장할 메모리 할당
        char *compressed_data = new char[max_dst_size];

        // 데이터 압축 실행
        size_t compressed_size = LZ4F_compressFrame(compressed_data, max_dst_size, buffer, read_size, NULL);
        
        // 압축 에러 확인
        if (LZ4F_isError(compressed_size)) {
            // 에러가 발생했을 경우 에러 메시지 출력
            std::cerr << "Failed to compress data: " << LZ4F_getErrorName(compressed_size) << std::endl;
            // 할당된 메모리 해제
            delete[] compressed_data;
            i ++;
            continue;
        }

        // 압축된 데이터를 파일에 쓰기
        ofs.write(compressed_data, compressed_size);

        // 메모리 해제
        delete[] compressed_data;
    }
    // 성공적으로 함수를 종료하고 0을 반환
    return 0;
}

// 디렉토리를 tar 압축 파일로 만드는 함수
int create_tar(const std::string &dir_path) {
    // tar 명령어를 이용하여 디렉토리를 tar 파일로 만드는 명령어 생성
    std::string command = "tar -cf request.tar " + dir_path + " 2> /dev/null";
    // 시스템 명령어 실행
    int result = std::system(command.c_str());
    if (result != 0) {
        return -1;
    }
    return compress_file("request.tar"); // lz4 압축파일 생성
}

void make_file(int server_socket, bool *isLoop) { // 함수 정의. send_file 함수는 서버 소켓과 루프 실행 여부를 인자로 받습니다.
    fd_set readfds; // 소켓 I/O를 위한 파일 디스크립터 세트를 선언합니다.
    struct timeval timeout; // select 함수의 타임아웃을 설정하기 위한 구조체 선언
    char buffer[BUFFER_SIZE]; // 데이터를 읽고 쓰기 위한 버퍼 선언
    std::string request_message = "", dir_path = "", file_path = "resp_server.lz4", EOF_MSG = "<EOF>"; // 메시지, 경로 등을 저장할 문자열 변수 선언
    int tar_res; // tar 파일 생성 및 파일 압축 결과를 저장할 변수 선언
    ssize_t n; // read, write 함수의 반환값을 저장할 변수 선언
    
    while(*isLoop){ // 루프 실행 조건 하에
        FD_ZERO(&readfds); // 파일 디스크립터 세트를 초기화
        FD_SET(server_socket, &readfds); // 서버 소켓을 파일 디스크립터 세트에 추가

        timeout.tv_sec = 2; // select 함수의 타임아웃을 2초로 설정
        timeout.tv_usec = 0; 

        int activity = select(server_socket + 1 , &readfds , NULL , NULL , &timeout); // select 함수를 이용해 소켓의 상태를 검사

        if ((activity < 0) && (errno!=EINTR)) { // select 함수 오류 검사
            printf("select error"); 
        } 

        if (FD_ISSET(server_socket , &readfds)) { // 서버 소켓이 읽기 가능한 상태라면
            memset(buffer, 0 , sizeof(buffer));
            n = read(server_socket, buffer, sizeof(buffer)-1); // 서버 소켓에서 데이터를 읽어 버퍼에 저장
            if (n < 0){ // 읽기 오류 검사
                std::perror("read");
                std::cout << "Client request error\n";
                exit(1);
            }
            dir_path = buffer; // 읽은 데이터를 경로로 사용

            tar_res = create_tar(dir_path.c_str()); // tar 파일 생성
            if (tar_res == -1) { // tar 파일 생성 오류 검사
                request_message = "path_err";
                std::cout << request_message << std::endl;
                n = write(server_socket, request_message.c_str(), request_message.size()); // 오류 메시지를 서버에 전송
                if (n <= 0) { // 쓰기 오류 검사
                    if (n < 0) add_log("File_Send", "tar write Err");
                    exit(1);
                } 
                continue;
            } else if (tar_res == -2) { // lz4 파일 생성 오류 검사
                request_message = "lz4_err";
                std::cout << request_message << std::endl;
                n = write(server_socket, request_message.c_str(), request_message.size()); // 오류 메시지를 서버에 전송
                if (n <= 0) { // 쓰기 오류 검사
                    if (n < 0) add_log("File_Send", "lz4 write Err");
                    exit(1);
                }
                continue;
            }
            request_message = "comp_ok"; // 압축 성공 메시지 설정
            request_message += '\0';
            n = write(server_socket, request_message.c_str(), request_message.size()); // 서버에 압축 성공 메시지 전송
            if (n <= 0) { // 쓰기 오류 검사
                    if (n < 0) add_log("File_Send", "msg_send err");
                    exit(1);
            }
            add_log("File_Send", "file send start!");
            file_send(server_socket, file_path);

        }
    }
}


void file_send(int server_socket, std::string file_path){
    char buffer[BUFFER_SIZE];
    ssize_t n;
    std::ifstream file_to_send(file_path.c_str(), std::ios_base::binary); // 압축한 파일을 읽기 위해 스트림 열기
    memset(buffer, 0, sizeof(buffer));
    while (file_to_send.read(buffer, sizeof(buffer))) {
        // 파일 내용을 서버로 전송한다.
        n = write(server_socket, buffer, file_to_send.gcount());
        if (n <= 0) { // 쓰기 오류 검사
            if (n < 0) add_log("Account", "file send write err");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화
    }
    file_to_send.close(); // 파일 스트림을 닫는다.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string end_sign = "<!EOS!>";
    end_sign += '\0';
    n = write(server_socket, end_sign.c_str(), end_sign.size()); // EOS 신호를 전송한다.
    if (n <= 0) { // 쓰기 오류 검사
        if (n < 0) add_log("File_Send", "Eos mssage write err");
        exit(1); 
    }
    add_log("File_Send", "file send complete!");
}
