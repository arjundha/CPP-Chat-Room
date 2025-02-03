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
	}

	// std::cout override
	friend std::ostream& operator << (std::ostream& os, const Message<T>& message)
	{
		os << "Message ID:" << int(message.header.id) << " Size:" << message.header.size;
		return os;
	}

	// Overloads for treating the body vector like a stack

	// Push new data into a message
	template<typename DataType>
	friend Message<T>& operator << (Message<T>& message, const DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Cannot push data into message vector, type is too complex");

		size_t size = message.body.size(); // current size

		message.body.resize(message.body.size() + sizeof(DataType)); // resize for new data

		std::memcpy(message.body.data() + size, &data, sizeof(DataType)); // copy new data starting at end of old data

		message.header.size = message.size(); // now update the size in the header

		return message;
	}

	// Pull data out of a message
	template<typename DataType>
	friend Message<T>& operator >> (Message<T>& message, DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Cannot pull data into message vector, type is too complex");

		size_t size = message.body.size() - sizeof(DataType); // Size of data to be pulled out

		std::memcpy(&data, message.body.data() + size, sizeof(DataType)); // Copy the data out of message and into "data" var

		// Update sizes
		message.body.resize(size);
		message.header.size = message.size();

		return message;
	}

};

template <typename T>
class Connection;

template <typename T>
struct OwnedMessage {
	std::shared_ptr<Connection<T>> remote = nullptr;
	Message<T> message;

	friend std::ostream& operator << (std::ostream& os, const OwnedMesage<T>& message) {
		os << message.message;
		return os;
	}
};