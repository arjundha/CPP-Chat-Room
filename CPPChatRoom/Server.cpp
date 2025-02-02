#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


std::vector<char> vBuffer(20 * 1024);

void grabSomeData(asio::ip::tcp::socket &socket) {
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code error, std::size_t length) {
			if (!error) {
				std::cout << "\n\nRead: " << length << " bytes\n\n";
				for (int i = 0; i < length; i++) {
					std::cout << vBuffer[i];
				}
				grabSomeData(socket); // Since we are doing async, we will keep reading until there is no data left
			}
		});
}

int main() {

	asio::error_code error;

	asio::io_context context; // Interface for the platform -- unique instance
	asio::io_context::work idleWork(context); // Keep the context busy
	std::thread threaded_context = std::thread([&]() {context.run();}); // run the context in its own thread

	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", error), 80); // Endpoint specified at specific IP address

	asio::ip::tcp::socket socket(context); // Create a socket which needs an io_context to implement on
	socket.connect(endpoint, error); // Connect the socket to the endpoint

	if (error) {
		std::cout << "Error: " << error.message() << std::endl;
	}
	else {
		std::cout << "Socket connected successfully."<< std::endl;
	}

	if (socket.is_open()) {
		grabSomeData(socket);

		// Do something if the socket connected correctly (HTTP request)
		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"HOST: example.com\r\n"
			"Connection: close\r\n\r\n";

		// write_some tries to send/write as much of the data as possible, asio buffer is an array of bytes
		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), error);

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (threaded_context.joinable()) {
			threaded_context.join();
		}
		
	}

	return 0;
}