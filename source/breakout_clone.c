#include <inttypes.h>
#include <stdbool.h>

#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

#pragma warning(disable: 4820) // bytes padding added after data member

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920

#define NUMBER_OF_ROWS 8
#define BRICKS_PER_ROW 10
#define TOTAL_BRICKS (NUMBER_OF_ROWS * BRICKS_PER_ROW)
#define BRICK_PADDING 1
#define BRICK_WIDTH (WINDOW_WIDTH / BRICKS_PER_ROW) - BRICK_PADDING
#define BRICK_HEIGHT (WINDOW_HEIGHT / 4 / BRICKS_PER_ROW) - BRICK_PADDING

const char global_class_name[] = "breakout_window_class";

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
	HDC device_context;
	int width;
	int height;
	Pixel *pixels;
	BITMAPINFO bitmap_info;
	HBITMAP bitmap_handle;
	uint16_t bpp;
} frame = {0};

typedef struct
{
	int x;
	int y;
} Vec2;

typedef struct
{
	Colour colour;
	uint8_t height;
	bool is_alive;
	uint8_t padding;
	Vec2 position;
	uint8_t value;
	uint8_t width;
} Brick;

struct
{
	Brick bricks[TOTAL_BRICKS];
	bool is_running;
	HWND window_handle;
} game = {0};

const Colour COLOUR_BLACK = {.b = 0, .g = 0, .r = 0, .a = 255}; 
const Colour COLOUR_GREEN = {.b = 48, .g = 134, .r = 2, .a = 255};
const Colour COLOUR_ORANGE = {.b = 10, .g = 133, .r = 194, .a = 255};
const Colour COLOUR_RED = {.b = 10, .g = 30, .r = 163, .a = 255};
const Colour COLOUR_YELLOW = {.b = 41, .g = 194, .r = 194, .a = 255};

void make_bricks(void);

void render(void);

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
			HDC paint_dc = BeginPaint(window_handle, &paint);
			StretchDIBits
			(
				paint_dc,
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
	frame.width = WINDOW_WIDTH;
	frame.height = WINDOW_HEIGHT;
	frame.pixels = VirtualAlloc(NULL, 4 * frame.width * frame.height, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (frame.pixels == NULL)
	{
		FatalAppExit(0, "Framebuffer allocation failed!");
	}

	frame.bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	frame.bitmap_info.bmiHeader.biWidth = frame.width;
	frame.bitmap_info.bmiHeader.biHeight = frame.height;
	frame.bitmap_info.bmiHeader.biPlanes = 1;
	frame.bitmap_info.bmiHeader.biBitCount = frame.bpp;
	frame.bitmap_info.bmiHeader.biCompression = BI_RGB;
	frame.bitmap_info.bmiHeader.biSizeImage = 0;
	frame.bitmap_info.bmiHeader.biClrUsed = 0;
	frame.bitmap_info.bmiHeader.biClrImportant = 0;	
	
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	
	game.window_handle = CreateWindowEx(
		0, // extended window styles
		global_class_name,
		"Breakout Clone",
		WS_POPUP,
		CW_USEDEFAULT, // x
		CW_USEDEFAULT, // y
		screen_width,
		screen_height,
		NULL, // handle to parent window
		NULL, // menu
		instance,
		NULL // lpParam
	);
	
	if (game.window_handle == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	ShowCursor(0);
	
	make_bricks();
	
	ShowWindow(game.window_handle, SW_SHOWMAXIMIZED);
	UpdateWindow(game.window_handle);	
	
	frame.device_context = GetDC(game.window_handle);
	
	game.is_running = true;
	
	while (game.is_running)
	{
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		
		render();
	}	
	
	return 0;
}

void make_bricks(void)
{	
	// make them bottom up, two rows of yellow at the bottom, then two rows of green, two rows of orange and then two rows of red at the top
	Brick *brick = game.bricks;
	int yOffset = frame.height - (NUMBER_OF_ROWS * (BRICK_HEIGHT + BRICK_PADDING));
	
	for (int row = 0; row < NUMBER_OF_ROWS; ++row)
	{
		for (int column = 0; column < BRICKS_PER_ROW; ++column)
		{
			switch (row)
			{
				case 0: // fall-through
				case 1:
				{
					brick->colour = COLOUR_YELLOW;
					brick->height = BRICK_HEIGHT;
					brick->width = BRICK_WIDTH;
					brick->is_alive = true;
					brick->padding = BRICK_PADDING;
					brick->position.x = (column + brick->padding) + (column * brick->width);
					brick->position.y = (row + brick->padding) + (row * brick->height) + yOffset;
					brick->value = 1;					
				} break;
				case 2: // fall-through
				case 3:
				{
					brick->colour = COLOUR_GREEN;
					brick->height = BRICK_HEIGHT;
					brick->width = BRICK_WIDTH;
					brick->is_alive = true;
					brick->padding = BRICK_PADDING;
					brick->position.x = (column + brick->padding) + (column * brick->width);
					brick->position.y = (row + brick->padding) + (row * brick->height) + yOffset;
					brick->value = 3;
				} break;
				case 4: // fall-through
				case 5:
				{
					brick->colour = COLOUR_ORANGE;
					brick->height = BRICK_HEIGHT;
					brick->width = BRICK_WIDTH;
					brick->is_alive = true;
					brick->padding = BRICK_PADDING;
					brick->position.x = (column + brick->padding) + (column * brick->width);
					brick->position.y = (row + brick->padding) + (row * brick->height) + yOffset;
					brick->value = 5;
				} break;
				case 6: // fall-through
				case 7:
				{
					brick->colour = COLOUR_RED;
					brick->height = BRICK_HEIGHT;
					brick->width = BRICK_WIDTH;
					brick->is_alive = true;
					brick->padding = BRICK_PADDING;
					brick->position.x = (column + brick->padding) + (column * brick->width);
					brick->position.y = (row + brick->padding) + (row * brick->height) + yOffset;
					brick->value = 7;
				} break;
				default:
				{
					// error
					brick->is_alive = false;
				} break;
			}
			
			++brick;
		}
	}
}

void render(void)
{
	memset(frame.pixels, 0, frame.width * frame.height * sizeof(Pixel));

	Brick *brick = game.bricks;
	
	for (int brick_index = 0; brick_index < TOTAL_BRICKS; ++brick_index)
	{
		if (!brick->is_alive)
		{
			continue;
		}
		
		for (int y = 0; y < brick->height; ++y)
		{
			for (int x = 0; x < brick->width; ++x)
			{
				Pixel *pixel = frame.pixels + (frame.width * (brick->position.y + y)); // start of the brick in the y + current row of the brick to render
				pixel += brick->position.x + x;
				*pixel = brick->colour;
			}
		}
		
		++brick;
	}
	
	RECT client_rect;
	GetClientRect(game.window_handle, &client_rect);
	int window_width = client_rect.right - client_rect.left;
	int window_height = client_rect.bottom - client_rect.top;
	StretchDIBits
	(
		frame.device_context,
		0, // xDest
		0, // yDest
		window_width, // destWidth
		window_height, // destHeight
		0, // xSrc
		0, // ySrc
		frame.width,
		frame.height,
		frame.pixels,
		&frame.bitmap_info,
		DIB_RGB_COLORS,
		SRCCOPY
	);	
}