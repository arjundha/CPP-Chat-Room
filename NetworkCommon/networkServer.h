#pragma once

#include "networkCommon.h"
#include "networkThreadSafeQueue.h"
#include "networkMessage.h"
#include "networkConnection.h"

template<typename T>
class ServerInterface {
public:
	ServerInterface(uint16_t port) : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {

	};

	virtual ~ServerInterface() {
		stop();
	};

	bool start() {
		try {
			waitForClientConnection();
			m_threadContext = std::thread([this]() {
				m_asioContext.run();
				});
		}
		catch (std::exception& e) {
			std::cerr << "[Server] [ERROR] Exception in networkServer: " << e.what() << "\n";
			return false;
		}
		std::cout << "[Server] Server Started!\n";
		return true;
	};

	void stop() {
		m_asioContext.stop();
		if (m_threadContext.joinable()) {
			m_threadContext.join();
		}
		std::cout << "[Server] Server stopped\n";
	};

	// async function
	void waitForClientConnection() {
		m_asioAcceptor.async_accept([this](std::error_code error, asio::ip::tcp::socket socket) {
			if (error) {
				std::cout << "[Server] [ERROR] Server encounterd new connection error: " << error.message() << "\n";
			}
			else {
				std::cout << "[Server] Server establised new connection at remote endpoint: " << socket.remote_endpoint() << "\n";

				// Make a connection to handle the new client that wants to connect
				// We tell this connection that it is owned by a server
				std::shared_ptr<Connection<T>> new_connection = std::make_shared<Connection<T>>(Connection<T>::Owner::Server, m_asioContext, std::move(socket), m_qMessagesIn);

				// Give the user server a chance to deny connection
				if (!onClientConnect(new_connection))
				{
					std::cout << "[Server] Connection denied -- Client could not connect\n";
				}
				else
				{
					// Client connected, so we put them in our deque
					m_deqConnections.push_back(std::move(new_connection));

					m_deqConnections.back()->connectToClient(nIDCounter++);

					std::cout << "[" << m_deqConnections.back()->getID() << "] Connected!!!\n";
				}
			}

			// Get context primed for accepting another connection by re-registering an async task
			waitForClientConnection();
			});

	};

	// message a specific client
	void messageClient(std::shared_ptr<Connection<T>> client, const Message<T>& message) {
		if (client && client->isConnected()) {
			client->send(message);
		}
		else {
			// Client must have disconnected
			onClientDisconnect(client);
			client.reset();
			m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
		}

	};

	// message all connected clients, but can ignore specific clients
	void messageAllClients(const Message<T>& message, std::shared_ptr<Connection<T>> pIgnoredClient = nullptr) {
		bool bInvalidClientExists = false; // boolean for if we have encountered a disconnected client

		for (auto& client : m_deqConnections)
		{
			// Check if the client is connected
			if (client && client->isConnected())
			{
				// Are they ignored? TODO: finalize if I want this 
				if (client != pIgnoredClient) {
					client->send(message);
				}
			}
			else
			{
				// Couldn't verify clients connection so we need to disconnect them
				onClientDisconnect(client);
				client.reset();

				// Set this flag to then remove dead clients from container
				bInvalidClientExists = true;
			}
		}

		// Only iterate through our deque if we need to, and only do it once
		if (bInvalidClientExists) {
			m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
		}
	};

	// size_t is unsigned, so setting it to -1 actually sets it to highest value it can be
	void update(size_t nMaxMessages = -1, bool bShouldWait = false) {
		if (bShouldWait) {
			m_qMessagesIn.wait();
		}
		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
			// get the first message
			auto message = m_qMessagesIn.pop_front();
			onMessage(message.remote, message.message);
			nMessageCount++;
		}
	};

protected:
	// Called whenever a client connects to the server
	virtual bool onClientConnect(std::shared_ptr<Connection<T>> client) {
		return false;
	};

	// Called whenever a client disconnects from the server
	virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client) {

	};

	// Called when the server receives a message
	virtual void onMessage(std::shared_ptr<Connection<T>> client, Message<T>& message) {

	};

	/*
	* Member variables
	*/
	// TSQ for messages
	ThreadSafeQueue<OwnedMessage<T>> m_qMessagesIn;

	// Deque of valid connections
	std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

	asio::io_context m_asioContext;
	std::thread m_threadContext;

	asio::ip::tcp::acceptor m_asioAcceptor;

	// Client ID identifiers
	uint32_t nIDCounter = 10000;
};