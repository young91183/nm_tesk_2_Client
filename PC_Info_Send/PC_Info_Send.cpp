#include "PC_Info_Send.h"


// CPU 정보 추출
nlohmann::json getCPUInfo() {
	std::ifstream cpuInfo("/proc/cpuinfo"); // 리눅스의 /proc/cpuinfo 정보를 추출
	std::string line;
	nlohmann::json json, processorjson; // 전체 cpu 정보를 저장할 객체와 processor별 정보를 담아 둘 객체 선언

	int i=-100, processor = 0;
	while (std::getline(cpuInfo, line)) { // 문장 마지막까지 개행문자를 기준으로 한줄씩 추출 

		if (i==-100){
			i = 0;
			continue;
		}

		std::string key, value;
		int colon = 0;
		for (i = 0;i<line.size();i++){
			if(line[i] == ':'){
				colon = i;
				break;
			}
		}

		try{
			key = line.substr(0, colon);
			value = line.substr(colon +1);
		} catch (std::out_of_range& e){
			continue;
		}
		for (i = 0; i<key.size();i++){
			if(key[i] == '	'){ // key 뒤에 tap이 포함된 경우 삭제
				colon = i;
				break;
			} else if (key[i] == ' ')key[i]='_'; // key의 중간에 띄어쓰기로 작성되어 있는 경우 '_'로 대체
		}
		key = key.substr(0, colon); // key의 필요한 부분만 추출

		try {
			for (i = 0; i<value.size();i++){
				if(value[i] != ' '){ // value 앞에 붙어있는 ' '들을 제거하기 위함
					colon = i;
					break;
				}
			}
			value = value.substr(colon); // vlaue값 필요한 부분만 추출
		} catch (std::out_of_range& e){  //power_management 정보가 NULL 값인 경우가 많아 예외처리
			processorjson[key] = "NULL";
			continue;
		}
		if(key == "processor"){
			json[processor] = processorjson;
			nlohmann::json processorjson;
			processor ++;
		} else {
			processorjson[key] = value;
		}
	}
	json[processor] = processorjson;
	cpuInfo.close();
	return json;
}


// Disk 정보 추출
nlohmann::json getDiskInfo(){
	std::ifstream diskStats("/proc/diskstats"); // 리눅스 /proc/diskstats 정보 추출
	nlohmann::json json, proc_disk;
	struct statvfs stat; // disk 정보 계산을 위해 시스템 정보 저장
	int cnt = 0;
	std::string key = "", value = "";
	std::string line, word;

	// /proc/diskstats의 각 필드의 정보를 하나하나씩 json 형태로 가공
	while (std::getline(diskStats, line)) {
		std::stringstream lineStream(line);
		while (std::getline(lineStream, word,' ')) {
			if(word == " " || word == "" || word == "	") continue;
			else if(cnt < 2) {
				key += word;
				cnt += 1;
				if(cnt == 1) key += "_";
			}
			else value += word + " ";
		}
		proc_disk[key] = value;
		key = "";
		value = "";
		cnt = 0;
	}
	json["disk_stats"] = proc_disk;
	diskStats.close();
	
	double totalSpace = ((stat.f_blocks*stat.f_frsize)/1024)/1024; // 전체 공간의 크기
	double freeSpace = ((stat.f_bavail * stat.f_frsize)/1024)/1024; // 가용 가능한 공간의 크기
	double blockSize = stat.f_frsize/1024; // 시스템이 사용 중인 블록 사이즈
	double usedSize = (((stat.f_blocks - stat.f_bfree)*stat.f_bsize)/1024)/1024; // 사용중인 공간의 크기
	json["total_space"] = totalSpace;
	json["free_space"] = freeSpace;
	json["block_size"] = blockSize;
	json["used_size"] = usedSize;
	
	return json;
}


// Memory 정보 추출
nlohmann::json getMemoryInfo() {
	std::ifstream memInfo("/proc/meminfo"); // /proc/meminfo에서 정보 추출
	std::string line;
	nlohmann::json json;
	int i;

	while (std::getline(memInfo, line)) { // memInfo 정보를 개행문자를 기준으로 분할해 처리
		std::istringstream iss(line);
		std::string key, value, kb;

		if (!(iss >> key >> value)) continue;
		key.pop_back(); // ':' 삭제

		// key이름에 ()처리된 경우 _로 대체
		for(i=0 ; i<key.size() ; i++){
			if(key[i] == '(') key[i]='_';
			else if(key[i] ==')') key.pop_back();
		}

		json[key] = value;
	}

	memInfo.close();
	return json;
}


// NIC 정보 추출
nlohmann::json getNICInfo(std::string *client_ip) {
	nlohmann::json json, netJson, rtJson, nullJson ;
	char buffer[128];
	std::string nicInfo = "" ;
	std::string line, word, f_key, key, value, rtKey;
	FILE* pipe = popen("ifconfig", "r"); // ifconfig 정보 추출

	int odd = 0, flag = 0, i = 0, rtSign = 0, cnt = 0, ip_cnt = 0;

	while (fgets(buffer, sizeof buffer, pipe) != NULL) {
		nicInfo += buffer;
	}
	pclose(pipe);

	std::stringstream ss(nicInfo);
	while (std::getline(ss, line)) { // 개행문자를 기준으로 ifconfig 정보를 분할해 한줄 씩 처리
		std::stringstream lineStream(line);
		while (std::getline(lineStream, word, ' ')){ // ' '를 기준으로 line을 한번더 분할해 요소 추출
			if(word == "" || word.front() == '(' ||  word.back() == ')')continue;
			else if (word == "inet6") break;

			if(word.back() == ':'){ // 네트워크 종류 별로 구분하기 위해 ':' 으로 끝나는 키워드 선별
				word.pop_back();
				if (cnt > 0){ // 새로 네트워크 류가 바뀐 경우
					netJson["TX"] = rtJson; // 이전 TX값을 저장
					json[f_key] = netJson;
					rtJson = nullJson;
					netJson = nullJson ;
					rtSign = 0;
					f_key = word; // 네트워크 이름 저장
				}else f_key = word; // 최초 네트워크 이름 저장
				cnt ++;
				flag = 1; // 네트워크 바로 다음에 등장하는 flag 처리를 위한 신호
				continue;
			}

			if (flag == 1 && word != "" ) { // flag 처리 위한 과정
				int ptr = 0;
				for (i = 0; i< word.size() ; i++){
					if(word[i] == '='){ // '='을 기준으로 flag문장을 분할
						ptr = i;
						break;
					}
				}
				key = word.substr(0, ptr);
				value = word.substr(ptr+1);
				netJson[key] = value;
				flag = 0; 
				continue;
			} else if(word == "loop"){ // loop 요소는 값이 존재하지 않아 pass
				continue;
			// RX, TX는 내용이 가변적이고 요소가 많아 json으로 정제해 각각 저장
			} else if(word == "RX" ||  word == "TX" ){ 
				rtKey = word;
				rtSign = 1;
				if (word =="TX"){
					netJson["RX"] = rtJson;
					nlohmann::json rtJson;
				}
			} else if(odd == 0) { // 지금 word가 key를 가지고 있다면
				key = word;
				odd = 1;
				continue;
			} else if(odd == 1){ // 지금 word가 value를 가지고 있다면
				value = word;
				odd = 0;
				if(key == "inet" && ip_cnt == 0) {
					*client_ip = value;
					ip_cnt = 1;
				}
				if(rtSign == 1){ // 만약 RX, TX 값을 추가중이라면  rtJson에 저장
					rtJson[key] = value;
					continue;
				}
				netJson[key] = value; // 아니라면 netJson[key]에 저장
			}
		}
	}
	netJson["TX"] = rtJson;
	json[f_key] = netJson;
	return json;
}


// 현재시간 추출 후 Date Time 형식으로 가공해 반환
std::string getCurrentDateTime() {
	auto now = std::chrono::system_clock::now();
	std::time_t now_t = std::chrono::system_clock::to_time_t(now);
	std::tm* now_tm = std::localtime(&now_t);

	char buffer[100];
	std::strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", now_tm); // date time 형태로 가공

	return std::string(buffer);
}


// account_info Table에 들어갈 정보를 정리해 json 형태로 가공
nlohmann::json getInfo(std::string ip, std::string id, std::string pw) {
	nlohmann::json json;
	int i;
	json["id"] = id;
	json["ip"] = ip;
	json["up_time"] = getCurrentDateTime();
	json["state"] = "login";
	json["password"]= pw;

	return json;
}


PC_Info_Client::PC_Info_Client() : server_socket(socket(AF_INET, SOCK_STREAM, 0)) { 
    if (server_socket < 0) {
        std::perror("socket");
        exit(1);
    }
}


int PC_Info_Client::start(std::string id, std::string pw, bool* isLoop) {
    std::string request_message, message;
    char buffer[1024];
    ssize_t n;
    std::chrono::system_clock::time_point start_time;
    std::chrono::nanoseconds runtime;

    while (*isLoop) {
        start_time = std::chrono::system_clock::now();
        server_socket = socket(AF_INET, SOCK_STREAM, 0);

        nlohmann::json pc_data;
        request_message = "pc_info";
        request_message += '\0';
        if (!connectToServer()) exit(1);
        n = write(server_socket, request_message.c_str(), request_message.size());
        if (n <= 0) {
            if (n < 0) add_log("PC_Info", "request mssage write err");
            return -1;
        }

        pc_data["cpu"] = getCPUInfo();
        pc_data["mem"] = getMemoryInfo();
        pc_data["disk"] = getDiskInfo();
        pc_data["nic"] = getNICInfo(&client_ip);
        pc_data["account_info"] = getInfo(client_ip, id, pw);
        std::string message = pc_data.dump();
		n = write(server_socket, message.c_str(), message.size());
		if (n <= 0) {
			if (n < 0) add_log("PC_Info", "Json send write err");
			return -1;
		}
		add_log("PC_Info", "Json send success!");
		close(server_socket);

        runtime = std::chrono::system_clock::now() - start_time;

        auto sleepTime = std::chrono::nanoseconds(4000000000)-runtime; // 멈출 시간
        auto end = std::chrono::system_clock::now() + sleepTime; // 깨어날 시각 계산
        while(std::chrono::system_clock::now() < end) {
            if(!*isLoop){ // 만약 중간에 isLoop가 꺼질 경우 종료
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    return 0;
}


bool PC_Info_Client::connectToServer() {
	sockaddr_in server_address{};
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		add_log("PC_Info", "Connect err");
		return false; // Connect 실패 시 false 반환
	}
	return true;
}


void pc_info_start(std::string id, std::string pw, bool * isLoop) {
	int pc_num = atoi(PC_NUM), err; // PC_NUM을 정수형으로 변환해 사용
	while(*isLoop) {
		auto now = std::chrono::system_clock::now(); // 현재시간 추출
		// 현재시간을 초단위로 변경해 계산이 가능하도록 변환
		auto now_in_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
		if (now_in_sec % 2 == pc_num % 2){ // 고유번호를 이용해 Client의 첫 요청을 분산 (숫자는 자유롭게 설정 가능)
			PC_Info_Client * pc_info_client = new PC_Info_Client();
			err = pc_info_client->start(id, pw, isLoop);
			if (err != 0){
				return;
			}
			delete pc_info_client;
			break;
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}
