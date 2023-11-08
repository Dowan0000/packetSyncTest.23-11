#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<SOCKET> clients;

//int reqNum[8] = { 0, };
//char buffer[9] = { reqNum[7], reqNum[6] ,reqNum[5] ,reqNum[4] ,reqNum[3] ,reqNum[2] ,reqNum[1] ,reqNum[0], 0 };

int StartCount;
bool isFirst = true;

thread_local int LId = 0;

//void packet()
//{
//	long long result, result0, result1, result2, result3, result4, result5, result6, result7;
//	result0 = reqNum[0];
//	result1 = reqNum[1] << 8;
//	result2 = reqNum[2] << 16;
//	result3 = reqNum[3] << 24;
//	result4 = reqNum[4] << 32;
//	result5 = reqNum[5] << 40;
//	result6 = reqNum[6] << 48;
//	result7 = reqNum[7] << 56;
//	
//	result = result0 + result1 + result2 + result3 + result4 + result5 + result6 + result7;
//	cout << "Result: " << result << endl;
//	
//}

void sendToAll(int id)
{
	LId = id;
	cout << "Thread " << LId << " Start" << endl;

	int reqNum[8] = { 0, };
	char buffer[10] = { reqNum[7], reqNum[6] ,reqNum[5] ,reqNum[4] ,reqNum[3] ,reqNum[2] ,reqNum[1] ,reqNum[0], 0, LId };

	//while (true)
	for(int i = 0; i < 100; i++)
	{
		if (clients.size() > 0)
		{
			for (auto& client : clients)
			{
				if (isFirst)
				{
					StartCount = GetTickCount();
					isFirst = false;
				}
				buffer[8] = (GetTickCount64() - StartCount) / 1000;
				int sendLen = send(client, buffer, sizeof(buffer), 0);
			}
				
			reqNum[0]++;
			buffer[7] = reqNum[0];
			for (int i = 0; i < 7; i++)
			{
				if (reqNum[i] > 255)
				{
					buffer[7 - i] = (reqNum[i] = 0);
					reqNum[i + 1]++;
					buffer[7 - i - 1] = reqNum[i + 1];
				}
			}
			
		}

		//packet();
		this_thread::sleep_for(16ms);
	}
}

int main()
{
	cout << "Server Start" << endl;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(SOCKADDR));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(9004);

	bind(listenSocket, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	listen(listenSocket, 0);


	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(SOCKADDR));
	int nSize = sizeof(SOCKADDR);
	clients.push_back(accept(listenSocket, (SOCKADDR*)&clientAddr, &nSize));

	//thread t(sendToAll);

	vector<thread> threads;
	int id = 0;
	for (int i = 0; i < 5; i++)
	{
		threads.push_back(thread(sendToAll, ++id));
	}
		

	while (true)
	{
		/*sockaddr_in clientAddr;
		memset(&clientAddr, 0, sizeof(SOCKADDR));
		int nSize = sizeof(SOCKADDR);
		clients.push_back(accept(listenSocket, (SOCKADDR*)&clientAddr, &nSize));*/
	}
	

	closesocket(listenSocket);
	for (auto& client : clients)
		closesocket(client);

	WSACleanup();

	return 0;
}