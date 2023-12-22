#ifndef FILE_SEND_H
#define FILE_SEND_H

#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstdlib>
#include "lz4frame.h"
#include "../PC_Info_Send/PC_Info_Send.h"
#include "../Add_Log/Add_Log.h"

int compress_file(const std::string &file_path); // tar 파일 lz4 압축하기
int create_tar(const std::string &dir_path); // tar 파일 만들기
void make_file(int server_socket, bool *isLoop); // 로그인에 사용한 소켓으로 파일 전송 대기
void file_send(int server_socket, std::string file_path); // 파일 전송만 담당

#endif // FILE_SEND_H
