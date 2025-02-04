#include <iostream>

#include <network.h>

enum class CustomMessage : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomClient : public ClientInterface<CustomMessage> {
public:
	void pingServer() {
		Message<CustomMessage> message;
		message.header.id = CustomMessage::ServerPing;

		// TODO: remove this
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		message << timeNow;
		send(message);
	}

};

int main() {
	CustomClient client;
	client.connect("127.0.0.1", 60000);

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };


	bool bQuit = false;
	while (!bQuit) {
		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}
		if (key[0] && !old_key[0]) client.pingServer();
		//if (key[1] && !old_key[1]) client.messageAll();
		if (key[2] && !old_key[2]) bQuit = true;

		for (int i = 0; i < 3; i++) {
			old_key[i] = key[i];
		};


		if (client.isConnected()) {
			if (!client.getIncoming().empty()) {
				auto message = client.getIncoming().pop_front().message;
				switch (message.header.id) {
				case CustomMessage::ServerAccept: {
					// Server accepted			
					std::cout << "Server accepted the connection\n";
				} break;


				case CustomMessage::ServerPing: {
					// Server has responded to a ping request
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					message >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				} break;

				case CustomMessage::ServerMessage: {
					uint32_t clientID;
					message >> clientID;
					std::cout << "Hello from user " << clientID << "]\n";
				}break;
				}
			}
		}
		else {
			std::cout << "Server Down \n";
			bQuit = true;
		}
	}
	return 0;
};