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

class CustomServer : public ServerInterface<CustomMessage> {
public:
	CustomServer(uint16_t port) :ServerInterface<CustomMessage>(port) {

	};

protected:

	virtual bool onClientConnect(std::shared_ptr<Connection<CustomMessage>> client) {
		// TODO: notify everyone when someone joins
		Message<CustomMessage> message;
		message.header.id = CustomMessage::ServerAccept;
		client->send(message);
		return true;
	};

	virtual void onClientDisconnect(std::shared_ptr<Connection<CustomMessage>> client) {
		std::cout << "[Server] Removing client with ID: " << client->getID() << "\n";
		// TODO: Notofy everyone when someone leaves
		//Message<CustomMessage> message;
		//message.header.id = CustomMessage::ServerMessage;
		//message << client->getID();
		//messageAllClients(message);
	};

	virtual void onMessage(std::shared_ptr<Connection<CustomMessage>> client, Message<CustomMessage>& message)
	{
		switch (message.header.id) {
		case CustomMessage::ServerPing: {
			std::cout << "[" << client->getID() << "]: Server Ping\n";

			// Bounce back to client
			client->send(message);
		} break;

		case CustomMessage::ServerMessage: {
			std::cout << "[Server]: User " << client->getID() << " has disconnected.\n";

			// Say bye bye
			message << client->getID();
			messageAllClients(message, client);
		} break;

		case CustomMessage::MessageWave: {
			std::cout << "[" << client->getID() << "]: Message Wave\n";

			// Make a message and send to all clients
			Message<CustomMessage> message;
			message.header.id = CustomMessage::MessageWave;
			message << client->getID();
			messageAllClients(message, client);
		} break;

		case CustomMessage::MessageAll: {
			std::cout << "[" << client->getID() << "]: Message All\n";

			// Add client ID to message
			message << client->getID();
			messageAllClients(message, client);
		} break;
		}
	};
};

int main() {
	CustomServer server(60000);
	server.start();
	while (1) {
		server.update(-1, true);
	}
	return 0;
};