#include <iostream>

#include <network.h>

enum class CustomMessage : uint32_t {
	Say,
	Shout
};

class CustomClient : public ClientInterface<CustomMessage> {
public:


};

int main() {
	CustomClient client;
	return 0;
}