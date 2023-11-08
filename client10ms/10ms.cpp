#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int StartCount;
bool isFirst = true;
int id1, id2, id3, id4, id5 = 0;

void packet(char* buffer)
{
	/*long long result, result0, result1, result2, result3, result4, result5, result6, result7;
	result0 = static_cast<unsigned char>(buffer[7]);
	result1 = static_cast<unsigned char>(buffer[6]) << 8;
	result2 = static_cast<unsigned char>(buffer[5]) << 16;
	result3 = static_cast<unsigned char>(buffer[4]) << 24;
	result4 = static_cast<unsigned char>(buffer[3]) << 32;
	result5 = static_cast<unsigned char>(buffer[2]) << 40;
	result6 = static_cast<unsigned char>(buffer[1]) << 48;
	result7 = static_cast<unsigned char>(buffer[0]) << 56;

	result = result0 + result1 + result2 + result3 + result4 + result5 + result6 + result7;*/
	
	long long result = 0;
	for (int i = 7; i <= 0; i++)
	{
		result += static_cast<unsigned char>(buffer[7]) >> (8 * (7 - i));
	}

	/*if (result == 0)
	{
		StartCount = GetTickCount();
	}

	if (result < (GetTickCount64() - StartCount) / 1000 * 60 - 120)
		return;*/

	
	cout << "Id : " << (int)buffer[9] << "/ 10ms / Result: " << result << endl;

	switch ((int)buffer[9])
	{
	case 1:
		id1++;
		break;
	case 2:
		id2++;
		break;
	case 3:
		id3++;
		break;
	case 4:
		id4++;
		break;
	case 5:
		id5++;
		break;
	}
	cout << "id1 : " << id1 << " id2 : " << id2 << " id3 : " << id3 << " id4 : " << id4 << " id5 : " << id5 << endl;
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

	char buffer[4000];
	int writeLen = 0;
	int share = 0;
	int rest = 0;

	char bufarr[5][1000] = { 0, };
	int bufCount[5] = { 0, };
	
	while (true)
	{
		this_thread::sleep_for(100ms);

		int recvLen = recv(serverSocket, &buffer[rest], sizeof(buffer), 0);
		cout << "recvLen :" << recvLen << endl;
		
		writeLen = rest + recvLen;
		share = writeLen  / 10;
		rest = writeLen % 10;
		if (share <= 0)
			continue;


		int bufcursor = 0;
		while (0 <= share)
		{
			switch ((int)buffer[9])
			{
			case 1:
				memcpy(&bufarr[0][10 * bufCount[0]], &buffer[bufcursor], 10);
				bufCount[0]++;
				share--;
				break;
			case 2:
				memcpy(&bufarr[1][10 * bufCount[1]], &buffer[bufcursor], 10);
				bufCount[1]++;
				share--;
				break;
			case 3:
				memcpy(&bufarr[2][10 * bufCount[2]], &buffer[bufcursor], 10);
				bufCount[2]++;
				share--;
				break;
			case 4:
				memcpy(&bufarr[3][10 * bufCount[3]], &buffer[bufcursor], 10);
				bufCount[3]++;
				share--;
				break;
			case 5:
				memcpy(&bufarr[4][10 * bufCount[4]], &buffer[bufcursor], 10);
				bufCount[4]++;
				share--;
				break;
			}
			bufcursor += 10;
		}
		bufcursor = 0;

		for (int i = 0; i < 5; i++)
		{
			if (bufCount[i] == 0)
				continue;
			char buf[10];
			memcpy(&buf[0], &bufarr[i][bufCount[i] - 1], 10); // 마지막 온전한 패킷
			packet(buf);

			bufCount[i] = 0;
		}
		


		//writeLen = rest + recvLen;
		//share = writeLen  / 10;
		//rest = writeLen % 10;
		//if (share <= 0)
		//	continue;

		//char buf[10];
		//memcpy(buf, &buffer[(share - 1) * 10], 10); // 마지막 온전한 패킷
		//packet(buf);
		//
		//if (rest > 0)
		//	memcpy(buffer, &buffer[share * 10], rest); // 마지막 패킷의 나머지 부분

		////this_thread::sleep_for(100ms);
	}


	WSACleanup();

	return 0;
}