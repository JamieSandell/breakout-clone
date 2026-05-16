#include <inttypes.h>
#include <stdbool.h>

#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

const char global_class_name[] = "breakout_window_class";
static HDC device_context;

struct
{
	int width;
	int height;
	uint32_t *pixels;
	BITMAPINFO bitmap_info;
	HBITMAP bitmap_handle;
	HDC device_context;
} frame = {0};

struct
{
	bool is_running;
} game = {0};

LRESULT CALLBACK window_procedure
(
	HWND window_handle,
	UINT message,
	WPARAM w_param,
	LPARAM l_param
)
{
	LRESULT result = 0;
	
	switch(message)
	{
		case WM_ACTIVATEAPP:
		{
		} break;
		case WM_CLOSE:
		{
			DestroyWindow(window_handle);
		} break;
		case WM_DESTROY:
		{
			game.is_running = false;
			PostQuitMessage(0);
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {0};
			device_context = BeginPaint(window_handle, &paint);
			StretchDIBits
			(
				device_context,
				paint.rcPaint.left, // xDest
				paint.rcPaint.top, // yDest
				paint.rcPaint.right - paint.rcPaint.left, // destWidth
				paint.rcPaint.bottom - paint.rcPaint.top, // destHeight
				0, // xSrc
				0, // ySrc
				frame.width,
				frame.height,
				frame.pixels,
				&frame.bitmap_info,
				DIB_RGB_COLORS,
				SRCCOPY
			);
			EndPaint(window_handle, &paint);			
		} break;
		case WM_SIZE:
		{
			if (VirtualAlloc(NULL, 4 * frame.width * frame.height, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) == NULL)
			{
				FatalAppExit(0, "Framebuffer allocation failed!");
			}
		} break;
		default:
		{
			result = DefWindowProc(window_handle, message, w_param, l_param);
		} break;
	}
	
	return result;
}

int WINAPI WinMain
(
	HINSTANCE instance,
	HINSTANCE previous_instance,
	char *command_line,
	int command_show
)
{
    WNDCLASSEX window_class = {0};
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
	
	frame.width = 480;
	frame.height = 270;
	
	window_handle = CreateWindowEx(
		WS_EX_CLIENTEDGE,	
		global_class_name,
		"Breakout Clone",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, // x
		CW_USEDEFAULT, // y
		frame.width,
		frame.height,
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
	
	game.is_running = true;
	
	while (game.is_running)
	{
		while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}	
	
	return 0;
}