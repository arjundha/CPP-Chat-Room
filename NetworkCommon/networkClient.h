#pragma once

#include "networkCommon.h"
#include "networkThreadSafeQueue.h"
#include "networkMessage.h"
#include "networkConnection.h"

template <typename T>
class ClientInterface {
public:
	ClientInterface() : m_socket(m_context) {
		// initialize the socket with the context
		//m_socket = asio::ip::tcp::socket(m_context); same as using member initializer list
	}
	virtual ~ClientInterface() {
		disconnect();
	}

	// Try to connect to a given host with port
	bool connect(const std::string& host, const uint16_t port) {
		try {
			// Try to reolsve host inot endpoints
			asio::ip::tcp::resolver resolver(m_context);
			asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

			// Connect in here
			m_connection = std::make_unique<Connection<T>>(
				Connection<T>::Owner::Client,
				m_context,
				asio::ip::tcp::socket(m_context),
				m_qMessagesIn);

			// Connect to server via Connection obj
			m_connection->connectToServer(endpoints);

			// Start context in a thread
			thread_context = std::thread([this]() {
				m_context.run();
				});
		}
		catch (std::exception& e) {
			std::cerr << "Exception in networkClient: " << e.what() << "\n";
			return false;
		}
		return true;
	}

	// Disconnect from server
	void disconnect() {
		if (isConnected()) {
			m_connection->disconnect();
		}
	}

	bool isConnected() {
		return m_connection ? m_connection->isConnected() : false;
	}

	ThreadSafeQueue<OwnedMessage<T>>& getIncoming() {
		return m_qMessagesIn;
	}

	// Send message to server
	void send(const Message<T>& message) {
		if (isConnected()) {
			m_connection->send(message);
		}
	};

protected:
	// asio context for data transfer
	asio::io_context m_context;
	std::thread thread_context; // context needs to be in a thread
	asio::ip::tcp::socket m_socket; // socket connected to server
	std::unique_ptr<Connection<T>> m_connection; // Connection obj handles data transfer, hence unique_ptr

private:
	// TSQ of incoming messages
	ThreadSafeQueue<OwnedMessage<T>> m_qMessagesIn;
};