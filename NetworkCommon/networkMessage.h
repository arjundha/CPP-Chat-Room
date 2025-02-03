#pragma once
#include "networkCommon.h"


template <typename T>
// Everything is public, so using structs instead of classes
struct message_header {
	T id{};
	uint32_t size = 0;
};

template <typename T>
struct message {
	message_header<T> header{};
	std::vector<uint8_t> body;

	size_t size() const {
		return sizeof(message_header<T>) + body.size();
	}

	// std::cout override
	friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
	{
		os << "Message ID:" << int(msg.header.id) << " Size:" << msg.header.size;
		return os;
	}

	// Overloads for treating the body vector like a stack

	// Push new data into a message
	template<typename DataType>
	friend message<T>& operator << (message<T>& msg, const DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Cannot push data into message vector, type is too complex");

		size_t size = msg.body.size(); // current size

		msg.body.resize(msg.body.size() + sizeof(DataType)); // resize for new data

		std::memcpy(msg.body.data() + size, &data, sizeof(DataType)); // copy new data starting at end of old data

		msg.header.size = msg.size(); // now update the size in the header

		return msg;
	}

	// Pull data out of a message
	template<typename DataType>
	friend message<T>& operator >> (message<T>& msg, DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Cannot pull data into message vector, type is too complex");

		size_t size = msg.body.size() - sizeof(DataType); // Size of data to be pulled out

		std::memcpy(&data, msg.body.data() + size, sizeof(DataType)); // Copy the data out of message and into "data" var

		// Update sizes
		msg.body.resize(size);
		msg.header.size = msg.size();

		return msg;
	}

};
