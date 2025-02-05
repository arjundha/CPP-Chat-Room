#include <iostream>

#include <network.h>

enum class CustomMessage : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageWave,
	MessageAll,
	ServerMessage,
};

class CustomClient : public ClientInterface<CustomMessage> {
public:
	void pingServer() {
		Message<CustomMessage> message;
		message.header.id = CustomMessage::ServerPing;
		send(message);
	}

	void wave()
	{
		Message<CustomMessage> message;
		message.header.id = CustomMessage::MessageWave;
		send(message);
	}

	void messageAll(std::string input) {
		Message<CustomMessage> message;
		message.header.id = CustomMessage::MessageAll;
		message << input;
		send(message);
	}

	void disconnect() {
		Message<CustomMessage> message;
		message.header.id = CustomMessage::ServerMessage;
		send(message);
	}

};

int main() {
	CustomClient client;
	client.connect("127.0.0.1", 60000);

	bool bQuit = false;

	// Thread for user input
	std::thread inputThread([&]() {
		std::string input;
		std::cout << "\nQuick commands: (1: Ping, 2: Wave, 3: Quit). Otherwise, type a message: \n";
		while (!bQuit) {
			std::getline(std::cin, input);

			if (input == "1") {
				client.pingServer();
			}

			else if (input == "2") {
				client.wave();
				std::cout << "You waved!\n";
			}

			else if (input == "3") {
				client.disconnect();
				bQuit = true;
			}
			else {
				client.messageAll(input);
			}
		}
		});

	while (!bQuit) {
		if (client.isConnected()) {
			if (!client.getIncoming().empty()) {
				auto message = client.getIncoming().pop_front().message;
				switch (message.header.id) {
				case CustomMessage::ServerAccept: {
					// Server accepted			
					std::cout << "Server accepted your connection. Start chatting!\n";
				} break;

				case CustomMessage::ServerPing: {
					// Server has responded to a ping request to verify still connected
					std::cout << "Ping: " << "Server responded to ping" << "\n";
				} break;

				case CustomMessage::MessageWave: {
					uint32_t clientID;
					message >> clientID;
					std::cout << "User [" << clientID << "] waved!\n";
				} break;

				case CustomMessage::MessageAll: {
					uint32_t clientID;
					std::string output;
					message >> clientID;
					message >> output;
					std::cout << "[" << clientID << "]: " << output << "\n";
				} break;

				case CustomMessage::ServerMessage: {
					uint32_t clientID;
					message >> clientID;
					std::cout << "User [" << clientID << "] disconnected.\n"; //TODO
				}break;
				}
			}
		}
		else {
			std::cout << "Server Down \n";
			bQuit = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent CPU overuse
	}

	return 0;
};