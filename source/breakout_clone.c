#include <inttypes.h>
#include <stdbool.h>

#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

#pragma warning(disable: 4820) // bytes padding added after data member 

const char global_class_name[] = "breakout_window_class";
static HDC device_context;

typedef union
{
	uint32_t raw;
	struct
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
} Pixel, Colour;

struct
{	
	int width;
	int height;
	Pixel *pixels;
	BITMAPINFO bitmap_info;
	HBITMAP bitmap_handle;
	HDC device_context;
	uint16_t bpp;
} frame = {0};

struct
{
	bool is_running;
} game = {0};

const Colour COLOUR_BLACK = {.b = 0, .g = 0, .r = 0, .a = 255}; 
const Colour COLOUR_BLUE = {.b = 255, .g = 0, .r = 0, .a = 255};
const Colour COLOUR_GREEN = {.b = 0, .g = 255, .r = 0, .a = 255};
const Colour COLOUR_RED = {.b = 0, .g = 0, .r = 255, .a = 255};

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
		case WM_CLOSE:
		{
			DestroyWindow(window_handle);
		} break;
		case WM_DESTROY:
		{
			game.is_running = false;
			PostQuitMessage(0);
		} break;
		case WM_KEYDOWN:
		{
			switch (w_param)
			{
				case VK_ESCAPE:
				{
					DestroyWindow(window_handle);
				} break;
				default:
				{
					
				} break;
			}
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
	
	frame.bpp = 32;
	frame.width = 480;
	frame.height = 270;
	frame.pixels = VirtualAlloc(NULL, 4 * frame.width * frame.height, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (frame.pixels == NULL)
	{
		FatalAppExit(0, "Framebuffer allocation failed!");
	}

	frame.bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFO);
	frame.bitmap_info.bmiHeader.biWidth = frame.width;
	frame.bitmap_info.bmiHeader.biHeight = frame.height;
	frame.bitmap_info.bmiHeader.biPlanes = 1;
	frame.bitmap_info.bmiHeader.biBitCount = frame.bpp;
	frame.bitmap_info.bmiHeader.biCompression = BI_RGB;
	frame.bitmap_info.bmiHeader.biSizeImage = 0;
	frame.bitmap_info.bmiHeader.biClrUsed = 0;
	frame.bitmap_info.bmiHeader.biClrImportant = 0;

	// pixel format is BGRA
	for (int y = 0; y < frame.height; ++y)
	{
		for (int x = 0; x < frame.width; ++x)
		{
			Pixel *pixel = frame.pixels + x + (frame.width * y);
			*pixel = COLOUR_BLACK;
		}
	}
	
	window_handle = CreateWindowEx(
		0, // extended window styles
		global_class_name,
		"Breakout Clone",
		WS_POPUP,
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
	
	ShowWindow(window_handle, WS_MAXIMIZED);
	UpdateWindow(window_handle);
	
	game.is_running = true;
	
	while (game.is_running)
	{
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}	
	
	return 0;
}