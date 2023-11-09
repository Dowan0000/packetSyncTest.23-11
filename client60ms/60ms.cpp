#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int StartCount;
bool isFirst = true;

void packet(char* buffer)
{
	long long result, result0, result1, result2, result3, result4, result5, result6, result7;
	result0 = static_cast<unsigned char>(buffer[7]);
	result1 = static_cast<unsigned char>(buffer[6]) << 8;
	result2 = static_cast<unsigned char>(buffer[5]) << 16;
	result3 = static_cast<unsigned char>(buffer[4]) << 24;
	result4 = static_cast<unsigned char>(buffer[3]) << 32;
	result5 = static_cast<unsigned char>(buffer[2]) << 40;
	result6 = static_cast<unsigned char>(buffer[1]) << 48;
	result7 = static_cast<unsigned char>(buffer[0]) << 56;

	result = result0 + result1 + result2 + result3 + result4 + result5 + result6 + result7;

	/*if (result == 0)
	{
		StartCount = GetTickCount();
	}

	if (result < (GetTickCount64() - StartCount) / 1000 * 60 - 120)
		return;*/

	
	cout << "/ 60ms / Result: " << result << endl;
}

int main()
{
	cout << "Client Start" << endl;
	this_thread::sleep_for(1s);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(SOCKADDR));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(9004);
	inet_pton(AF_INET, "127.0.0.1", &sockAddr.sin_addr);

	if (SOCKET_ERROR == connect(serverSocket, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)))
		return 0;

	while (true)
	{
		this_thread::sleep_for(16ms);

		char buffer[9];
		int recvLen = recv(serverSocket, buffer, sizeof(buffer), 0);

		/*if (isFirst)
		{
			StartCount = GetTickCount();
			isFirst = false;
		}

		if(buffer[8] < (GetTickCount64() - StartCount) / 1000 - 2)
			continue;*/

		packet(buffer);

		//this_thread::sleep_for(16ms);
	}


	WSACleanup();

	return 0;
}