#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

const char global_class_name[] = "breakout_window_class";

LRESULT CALLBACK window_procedure(
	HWND window_handle,
	UINT message,
	WPARAM w_param,
	LPARAM l_param)
{
	LRESULT result = 0;
	
	switch(message)
	{
		case WM_ACTIVATEAPP:
			break;
		case WM_CLOSE:
			DestroyWindow(window_handle);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			break;
		default:
			result = DefWindowProc(window_handle, message, w_param, l_param);
			break;
	}
	
	return result;
}

int WINAPI WinMain(
		HINSTANCE instance,
		HINSTANCE previous_instance,
		char *command_line,
		int command_show
	)
{
    WNDCLASSEX window_class = {};
	HWND window_handle;
	MSG message;
	
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.lpfnWndProc = window_procedure;
	window_class.hInstance = instance;
	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	//window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION); // TODO: Add a large icon
	window_class.lpszClassName = global_class_name;
	//window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // TODO: Add a small icon
	
	if (!RegisterClassEx(&window_class))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		
		return 0;
	}
	
	window_handle = CreateWindowEx(
		WS_EX_CLIENTEDGE,	
		global_class_name,
		"Breakout Clone",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, // x
		CW_USEDEFAULT, // y
		240, // width
		120, // height
		NULL, // handle to parent window
		NULL, // menu
		instance,
		NULL // lpParam
	);
	
	if (window_handle == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	ShowWindow(window_handle, command_show);
	UpdateWindow(window_handle);
	
	while(GetMessage(&message, NULL, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	
	return message.wParam;
}