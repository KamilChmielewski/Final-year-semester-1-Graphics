#include "Application.h"
#include <ctime>
#include <cstdio>
#define MS_PER_UPDATE 16.6666666667
#pragma enable_d3d11_debug_symbols

void GameLoopDelay(float frameStartTime)
{
	float frameProcessingTime, currentFrameTime;

	currentFrameTime = GetTickCount64();

	frameProcessingTime = currentFrameTime - frameStartTime;

	if (frameProcessingTime < 16.67f)
	{
		Sleep(16.67f - frameProcessingTime);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Application* theApp = new Application();

	std::clock_t start;
	float duration;

	start = std::clock();

	if (FAILED(theApp->Initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

	// Main message loop
	MSG msg = { 0 };

	double _previous = GetTickCount64();
	double _lag = 0.0;

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			bool handled = false;

			if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
			{
				handled = theApp->HandleKeyboard(msg);
			}
			else if (WM_QUIT == msg.message)
				break;

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_ESCAPE))
				exit(0);

			duration = (std::clock() - start) / (double)CLOCKS_PER_SEC; //Time the application has been running
			float frameStartTime = GetTickCount();
			theApp->Update(MS_PER_UPDATE / 1000, duration);

			theApp->Draw();

			GameLoopDelay(frameStartTime);

			//std::cout << "Time Running: " << duration << std::endl;
		}
	}

	delete theApp;
	theApp = nullptr;

	return (int)msg.wParam;
}