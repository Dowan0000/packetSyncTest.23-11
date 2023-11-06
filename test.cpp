#include <iostream>

using namespace std;

int reqNum1[8] = { 0, };
int i = 0;
char buffer1[8] = { i, reqNum1[6] ,reqNum1[5] ,reqNum1[4] ,reqNum1[3] ,reqNum1[2] ,reqNum1[1] ,reqNum1[0] };

int main_t()
{
    /*char buffer[8] = { 0, };
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 1;

    int value1 = static_cast<int>(buffer[0]);

    int value2 = static_cast<int>(buffer[1]) << 8;

    int value3 = static_cast<int>(buffer[2]) << 16;

    int result = value1 + value2 + value3;

    std::cout << "Result: " << result << std::endl;*/


    cout << static_cast<int>(buffer1[0]) << endl;
    reqNum1[0] = 1;
    cout << static_cast<int>(buffer1[0]) << endl;

	return 0;
}