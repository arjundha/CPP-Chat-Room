# C++ Chat Room
## Introduction
This is a simple chat service created with `C++` to demonstrate effective usage of `multithreading`, `Asio`, and `sockets`.

The application solution has two projects and one static header library.

When built and compiled, there will be two `.exe` applications, one for hosting a server (`CPPChatRoom.exe`), and another for "using" the chat room as a client (`NetworkClient.exe`).

The server and clients communicate through asynchronous messaging with sockets, fulfilled via Asio. Each client is also running in their own thread.


## Set-Up
The default set-up for a locally hosted chat room
1. Compile the projects
1. Run `CPPChatRoom.exe` to begin the server.
1. Run `NetworkClient.exe` to connect as a client.
1. Repeat the above step to simulate as many users as you require.
1. Start chatting!

If you instead wanted to connect to a hosted server, you would need to alter line 46 in `NetworkClient\client.cpp`. By default this is connecting to localhost.
## References
This was made by following along OneLoneCoder's tutorial: https://www.youtube.com/watch?v=2hNdkYInj4g&ab_channel=javidx9

This tutorial was amazing and a great intro to Asio (without Boost!) for anyone who hasn't used it.