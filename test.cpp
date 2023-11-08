#include <iostream>

using namespace std;


int main_t()
{
    int arr[5] = { 0, };

	int arrarr[3][6] = { {1, 2, 3, 4, 5, 6}, {6, 7, 8, 9, 10, 11}, {11,12,13,14,15,16} };

	memcpy(&arr[0], &arrarr[2][1], 5 * sizeof(int));

	for(int i = 0; i < 5; i++)
		cout << arr[i] << endl;

	return 0;
}