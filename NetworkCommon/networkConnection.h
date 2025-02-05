#pragma once

#include "networkCommon.h"
#include "networkThreadSafeQueue.h"
#include "networkMessage.h"

template<typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
	enum class Owner {
		Server,
		Client
	};

	Connection(
		Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThreadSafeQueue<OwnedMessage<T>>& qIncoming) :
		m_asio_context(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIncoming)
	{
		m_ownerType = parent;
	};

	virtual ~Connection() {

	};

	uint32_t getID() const {
		return id;
	};

	void connectToClient(uint32_t uid = 0) {
		if (m_ownerType == Owner::Server) {
			if (m_socket.is_open()) {
				id = uid;
				readHeader();
			}
		}
	};

	void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) { // called by clients 
		if (m_ownerType == Owner::Client) {
			asio::async_connect(m_socket, endpoints, [this](std::error_code error, asio::ip::tcp::endpoint endpoints) {
				if (!error) {
					readHeader();
				}
				});
		}
	};

	bool disconnect() { // called by clients or server 
		if (isConnected()) {
			asio::post(m_asio_context, [this]() {
				m_socket.close();
				});
		}
		return false;
	};

	bool isConnected() const {
		return m_socket.is_open();
	};

	void send(const Message<T>& message) {
		asio::post(m_asio_context, [this, message]() {
			bool bIsWritingMessage = !m_qMessagesOut.empty();
			m_qMessagesOut.push_back(message);

			// If asio was already busy writing messages, then dont do anything
			// If it hasn't started writing anything, then lets get it started
			if (!bIsWritingMessage) {
				writeHeader();
			}

			});
	};

private:
	// Read a Header async
	void readHeader() {
		// Asio waits for message header, we can prepare by allocating space for the header while waiting
		asio::async_read(m_socket, asio::buffer(&m_temporaryIncomingMessage.header, sizeof(MessageHeader<T>)), [this](std::error_code error, std::size_t size) {
			if (!error) {
				// Does the message have a body? If yes then we are going to allocate space for the body and prime asio to read it
				if (m_temporaryIncomingMessage.header.size > 8) {
					m_temporaryIncomingMessage.body.resize(m_temporaryIncomingMessage.header.size - 8);
					readBody();
				}
				else {
					// The message did not have a body, add it to the queue 
					addToIncomingMessageQueue();
				}
			}
			else {
				std::cout << "Failure reading Header for ID: " << id << "\n" << "Error: " << error.message() << "\n";
				m_socket.close();
			}
			});
	};

	// Read a message body async
	void readBody() {
		// We know at this point that the message had a header, so we just need to wait for the bytes for the body
		asio::async_read(m_socket, asio::buffer(m_temporaryIncomingMessage.body.data(), m_temporaryIncomingMessage.body.size()), [this](std::error_code error, std::size_t size) {
			if (!error) {
				addToIncomingMessageQueue();
			}
			else {
				std::cout << "Failure reading Body for ID: " << id << "\n" << "Error: " << error.message() << "\n";
				m_socket.close();
			}
			});
	};

	// Adds incoming messages to the queue when they are received in full
	void addToIncomingMessageQueue() {
		if (m_ownerType == Owner::Server) {
			// OwnedMessage requires a shared pointer to the connection
			m_qMessagesIn.push_back({ this->shared_from_this(), m_temporaryIncomingMessage });
		}
		else {
			m_qMessagesIn.push_back({ nullptr, m_temporaryIncomingMessage });
		}
		// Reprime asio context
		readHeader();
	};

	// Write a header async
	void writeHeader() {
		asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)), [this](std::error_code error, std::size_t size) {
			if (!error) {
				// IF there is a body to send, then lets write it and send it
				if (m_qMessagesOut.front().body.size() > 0) {
					writeBody();
				}
				// Message was empty, lets get rid of it and try to do the next one
				else {
					m_qMessagesOut.pop_front();
					if (!m_qMessagesOut.empty()) {
						writeHeader();
					}
				}
			}
			else {
				std::cout << "Failure writing Header for ID: " << id << "\n" << "Error: " << error.message() << "\n";
				m_socket.close();
			}
			});
	};

	// Write a body async
	void writeBody() {
		asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
			[this](std::error_code error, std::size_t size) {
				if (!error) {
					m_qMessagesOut.pop_front();

					if (!m_qMessagesOut.empty())
					{
						writeHeader();
					}
				}
				else {
					std::cout << "Failure writing Body for ID: " << id << "\n" << "Error: " << error.message() << "\n";
					m_socket.close();
				}
			});
	};


protected:
	// ASIO stuff in here
	asio::ip::tcp::socket m_socket; // Unique for each socket to a remote
	asio::io_context& m_asio_context; // shared across instance
	ThreadSafeQueue<Message<T>> m_qMessagesOut; // messages to be sent to server from client
	ThreadSafeQueue<OwnedMessage<T>>& m_qMessagesIn; // holds all messages received from server. NOTE: must provide a queue

	Owner m_ownerType = Owner::Server;
	uint32_t id = 0;

	Message<T> m_temporaryIncomingMessage;
};