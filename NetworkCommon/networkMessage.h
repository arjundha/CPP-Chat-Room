#pragma once
#include "networkCommon.h"


template <typename T>
// Everything is public, so using structs instead of classes
struct MessageHeader {
	T id{};
	uint32_t size = 0;
};

template <typename T>
struct Message {
	MessageHeader<T> header{};
	std::vector<uint8_t> body;

	size_t size() const {
		return sizeof(MessageHeader<T>) + body.size();
	};

	// std::cout override
	friend std::ostream& operator << (std::ostream& os, const Message<T>& message)
	{
		os << "Message ID:" << int(message.header.id) << " Size:" << message.header.size;
		return os;
	};

	// Overloads for treating the body vector like a stack

	// Push new data into a message
	template<typename DataType>
	friend Message<T>& operator << (Message<T>& message, const DataType& data)
	{
		if constexpr (std::is_same<DataType, std::string>::value) {
			size_t size = message.body.size();  // Current size of message body

			// Resize to accommodate the string data (not including null terminator)
			message.body.resize(message.body.size() + data.size());

			// Copy the string's data into message.body
			std::memcpy(message.body.data() + size, data.data(), data.size());

			// Then store the string length
			uint32_t length = static_cast<uint32_t>(data.size());
			message << length; // Store length last (LIFO order)


		}
		else {
			static_assert(std::is_standard_layout<DataType>::value, "Cannot push data into message vector, type is too complex");

			size_t size = message.body.size(); // current size

			message.body.resize(message.body.size() + sizeof(DataType)); // resize for new data

			std::memcpy(message.body.data() + size, &data, sizeof(DataType)); // copy new data starting at end of old data
		}

		// Update header size to reflect body size
		message.header.size = message.size();
		return message;
	};

	// Pull data out of a message
	template<typename DataType>
	friend Message<T>& operator >> (Message<T>& message, DataType& data)
	{
		if constexpr (std::is_same<DataType, std::string>::value) {
			// Extract string length (which was stored last)
			uint32_t length;
			message >> length; // Read string length first (LIFO order)

			// Ensure there is data in the message
			if (message.body.empty()) {
				throw std::runtime_error("Message body is empty, cannot extract string");
			}

			// Compute starting index for the string (LIFO extraction)
			size_t size = message.body.size() - length;

			// Extract string data
			data.assign(message.body.begin() + size, message.body.end());

			// Clear the extracted data
			message.body.resize(size);
		}
		else {
			static_assert(std::is_standard_layout<DataType>::value, "Cannot pull data into message vector, type is too complex");

			size_t size = message.body.size() - sizeof(DataType); // Size of data to be pulled out

			std::memcpy(&data, message.body.data() + size, sizeof(DataType)); // Copy the data out of message and into "data" var

			// Update sizes
			message.body.resize(size);
		}
		message.header.size = message.size();
		return message;
	};

};

template <typename T>
class Connection;

/*
* These messages are associated with a connection.
*/
template <typename T>
struct OwnedMessage {
	std::shared_ptr<Connection<T>> remote = nullptr;
	Message<T> message;

	friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& message) {
		os << message.message;
		return os;
	};
};