#pragma once

#include "networkCommon.h"

template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;

	// Functions to mimic deque builtins
	const T& front() {
		std::scoped_lock lock(muxQueue);
		return deqQueue.front();
	}

	const T& back() {
		std::scoped_lock lock(muxQueue);
		return deqQueue.back();
	}


protected:
	std::mutex muxQueue;
	std::deque<T> deqQueue;
};