#include <stdio.h>

#include "Application.h"

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	Application app({ "SpotiVol 0.1", 200, 220 });
	app.Run();
	return 0;
}