CC = g++
CFLAGS = -c -std=c++17
LFLAGS = -lpthread -llz4
OBJECTS = Account_Client.o PC_Info_Send.o File_Send.o Add_Log.o main.o

on_client: $(OBJECTS)
	$(CC) $(OBJECTS) -o on_client $(LFLAGS)

Account_Client.o: Account_Client/Account_Client.cpp
	$(CC) $(CFLAGS) Account_Client/Account_Client.cpp

PC_Info_Send.o: PC_Info_Send/PC_Info_Send.cpp
	$(CC) $(CFLAGS) PC_Info_Send/PC_Info_Send.cpp

File_Send.o: File_Send/File_Send.cpp
	$(CC) $(CFLAGS) File_Send/File_Send.cpp

Add_Log.o: Add_Log/Add_Log.cpp
	$(CC) $(CFLAGS) Add_Log/Add_Log.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm -rf *o
