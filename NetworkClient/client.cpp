#include <iostream>

#include <network.h>

enum class CustomMessage : uint32_t {
	Say,
	Shout
};

int main() {
	Message<CustomMessage> message;
	message.header.id = CustomMessage::Say;

	//int a = 1;
	//bool b = true;
	//float c = 3.1415f;
	//struct {
	//	float x;
	//	float y;
	//} d[5];
	//message << a << b << c << d;
	//a = 99;
	//b = false;
	//c = 100.01f;
	//message >> d >> c >> b >> a;
	return 0;
}