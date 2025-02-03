#pragma once

#include "networkCommon.h"
#include "networkThreadSafeQueue.h"
#include "networkMessage.h"

template<typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
	Connection() {

	}

	virtual ~Connection() {

	}

	bool connectToServer() { // called by clients 
		return false;
	}
	bool disconnect() { // called by clients or server 
		return false;
	}
	bool isConnected() const {
		return false;
	}

	bool send(const message<T>& message) {
		return false;
	}

protected:
	// ASIO stuff in here
	asio::ip::tcp::socket m_socket; // Unique for each socket to a remote
	asio::io_context&, m_asio_context; // shared across instance
	ThreadSafeQueue<Message<T>> m_qMessagesOut; // messages to be sent to server from client
	ThreadSafeQueue<OwnedMessage<T>>& m_qMessagesIn; // holds all messages received from server. NOTE: must provide a queue

};