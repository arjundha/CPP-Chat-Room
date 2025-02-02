#pragma once
#include "networkCommon.h"


template <typename T> 
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

};
