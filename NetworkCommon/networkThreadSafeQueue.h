#pragma once

#include "networkCommon.h"

template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;

	/*
	* Methods that mimic the std::deque implementation:
	*/
	// check the front of the deque
	const T& front() {
		std::scoped_lock lock(muxQueue); // locks the muxQueue so its only modified by one thread (unlocks when leaves scope)
		return deqQueue.front();
	}

	// check the back of the deque
	const T& back() {
		std::scoped_lock lock(muxQueue);
		return deqQueue.back();
	}


	// Add an item to the front
	void push_front(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_front(std::move(item)); // moves item into the front of the deqQueue, prevents making copies via "move"
	}

	// Add an item to the back
	void push_back(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_back(std::move(item));
	}

	// Returns the size of the queue
	size_t count()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.size();
	}

	// checks if the queue isEmpty
	bool empty() {
		std::scoped_lock lock(muxQueue);
		return deqQueue.empty();
	}

	// Empties out all values in the deque
	void clear() {
		std::scoped_lock lock(muxQueue);
		deqQueue.empty();
	}

	// Remove and return the front value
	T pop_front()
	{
		std::scoped_lock lock(muxQueue);
		auto item = std::move(deqQueue.front()); // using "move" to prevent making copies
		deqQueue.pop_front(); // get rid of front value
		return item;
	}

	// Remove and return the back value
	T pop_back()
	{
		std::scoped_lock lock(muxQueue);
		auto item = std::move(deqQueue.back());
		deqQueue.pop_back();
		return item;
	}


protected:
	std::mutex muxQueue;
	std::deque<T> deqQueue;
};