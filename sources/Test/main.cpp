#include <windows.h>
#include <vector>

import testmodule;


struct local_vec
{
	int x;
	int y;
};

int WinMain(HINSTANCE,
	HINSTANCE,
	LPTSTR,
	int)
{
	std::vector<int> data = { 1,2,3,4,5 };

	std::vector<local_vec> data2 = { {1,2}, {3,4} };
	std::vector<module_vec> data3 = { {1,2}, {3,4} };

	return 0;
}