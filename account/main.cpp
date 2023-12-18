#include "Account_Client.h"

int main() {
	std::string id, pw, req;
	while(true) {
		Account_Client *account_client = new Account_Client();
		std::cout << "Request : ";
		std::cin >> req;
		if (req == "quit") break;

		std::cout << "ID : ";
		std::cin >> id;

		std::cout << "Password : ";
		std::cin >> pw;

		account_client->start(req, id, pw);
		delete account_client;
	}
	
	return 0;
}
