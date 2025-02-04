#include <iostream>
#include <network.h>

enum class CustomMessage : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomServer : public ServerInterface<CustomMessage> {
public:
	CustomServer(uint16_t port) :ServerInterface<CustomMessage>(port) {

	};

protected:

	virtual bool onClientConnect(std::shared_ptr<Connection<CustomMessage>> client)
	{
		//Message<CustomMessage> message;
		//message.header.id = CustomMessage::ServerAccept;
		//client->send(message);
		return true;
	};

	virtual void onClientDisconnect(std::shared_ptr<Connection<CustomMessage>> client)
	{
		//std::cout << "Removing client with ID: " << client->getID() << "\n";
	};

	virtual void onMessage(std::shared_ptr<Connection<CustomMessage>> client, Message<CustomMessage>& message)
	{
		//switch (message.header.id)
		//{
		//case CustomMessage::ServerPing:
		//{
		//	std::cout << "[" << client->getID() << "]: Server Ping\n";

		//	// Simply bounce message back to client
		//	client->send(message);
		//}
		//break;

		//case CustomMessage::MessageAll:
		//{
		//	std::cout << "[" << client->getID() << "]: Message All\n";

		//	// Construct a new message and send it to all clients
		//	Message<CustomMessage> message;
		//	message.header.id = CustomMessage::ServerMessage;
		//	message << client->getID();
		//	messageAllClients(message, client);

		//}
		//break;
		//}
	};
};

int main() {
	CustomServer server(60000);
	server.start();
	while (1) {
		server.update();
	}
	return 0;
};