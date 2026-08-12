#include <inttypes.h>
#include <stdbool.h>

#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

#pragma warning(disable: 4820) // bytes padding added after data member

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define RESOLUTION_WIDTH 480
#define RESOLUTION_HEIGHT 270

#define NUMBER_OF_ROWS 8
#define BRICKS_PER_ROW 14
#define TOTAL_BRICKS (NUMBER_OF_ROWS * BRICKS_PER_ROW)
#define BRICK_PADDING 2
#define BRICK_WIDTH 32
#define BRICK_HEIGHT 8

#define PLAYER_WIDTH 64
#define PLAYER_HEIGHT 8

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
	LARGE_INTEGER frequency;
	LARGE_INTEGER last_time;
	double frame_time;
	uint32_t fps;
	uint32_t frame_count;
} frame = {0};

typedef struct
{
	int x;
	int y;
} Vec2;

typedef struct
{
	uint8_t width;
	uint8_t height;
	Pixel *pixels;
	Vec2 position;
} Sprite;

typedef struct
{
	uint8_t width;
	uint8_t height;
	Colour colour;
	Vec2 position;
} RenderRect;

typedef struct
{
	bool is_alive;
	uint8_t padding;
	uint8_t value;
	RenderRect rect;
} Brick;

typedef struct
{
	RenderRect rect;
	uint16_t score;
} Player;

struct
{
	Brick bricks[TOTAL_BRICKS];
	bool is_running;
	Player player;
	HWND window_handle;
	Sprite score_sprite[3];
} game = {0};

const Colour COLOUR_BLACK = {.b = 0, .g = 0, .r = 0, .a = 255};
const Colour COLOUR_BLUE = {.b = 194, .g = 133, .r = 10, .a = 255};
const Colour COLOUR_GREEN = {.b = 48, .g = 134, .r = 2, .a = 255};
const Colour COLOUR_ORANGE = {.b = 10, .g = 133, .r = 194, .a = 255};
const Colour COLOUR_RED = {.b = 10, .g = 30, .r = 163, .a = 255};
const Colour COLOUR_TRANSPARENT = {.b = 186, .g = 123, .r = 215, .a = 255};
const Colour COLOUR_YELLOW = {.b = 41, .g = 194, .r = 194, .a = 255};

void load_bitmap_into_sprite(const char *file_name, Sprite *sprite);

void make_bricks(void);

void make_player(void);

void render(void);

void render_rect_to_frame(const RenderRect *rect);

void update(void);

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
	frame.width = RESOLUTION_WIDTH;
	frame.height = RESOLUTION_HEIGHT;
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
	make_player();
	load_bitmap_into_sprite("..\\assets\\sprites\\0.bmp", &game.score_sprite[0]);
		
	ShowWindow(game.window_handle, SW_SHOWMAXIMIZED);
	UpdateWindow(game.window_handle);	
	
	frame.device_context = GetDC(game.window_handle);
	
	game.is_running = true;
	
	QueryPerformanceFrequency(&frame.frequency);
	
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

void load_bitmap_into_sprite(const char *file_name, Sprite *sprite)
{
	HANDLE file_handle = CreateFile(
		file_name,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		FatalAppExit(0, "Failed to open file for reading.");
	}
	
	uint32_t file_size = GetFileSize(file_handle, NULL);
	
	HANDLE process_heap_handle = GetProcessHeap();
	
	if (process_heap_handle == NULL)
	{
		FatalAppExit(0, "Failed to get a handle to the process heap.");
	}
	
	char *raw_data = HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, file_size);
	unsigned long bytes_read;
	int32_t result = ReadFile(
		file_handle,
		raw_data,
		file_size,
		&bytes_read,
		NULL
	);
	
	if (result == 0)
	{
		FatalAppExit(0, "Failed to read file.");
	}
	
	uint16_t bf_type = *((uint16_t *)raw_data);
	
	if (bf_type != 0x4d42)
	{
		FatalAppExit(0, "Not a valid bitmap file.");
	}
	
	unsigned long offset = *((unsigned long *)(raw_data + 10));
	
	if (offset != 54)
	{
		FatalAppExit(0, "Bitmap should be 24bpp uncompressed.");
	}
	
	BITMAPINFOHEADER *info_header = (BITMAPINFOHEADER *)(raw_data + 40);
	sprite->height = (uint8_t)info_header->biHeight;
	sprite->width = (uint8_t)info_header->biWidth;
	
	unsigned long bytes_to_read = bytes_read - offset;
	
	for (unsigned long i = 0; i < bytes_to_read; i += 3)
	{
		uint32_t *current_pixel = (uint32_t *)(raw_data + offset + i);
		*(sprite->pixel) = *current_pixel;
		sprite->pixel->a = 255;
	}
	
	CloseHandle(file_handle);
}

void make_bricks(void)
{
	Brick *brick = game.bricks;

	int brick_spacing_x = BRICK_WIDTH + BRICK_PADDING;
	int brick_spacing_y = BRICK_HEIGHT + BRICK_PADDING;
	int y_offset = 160;

	for (int row = 0; row < NUMBER_OF_ROWS; ++row)
	{
		for (int column = 0; column < BRICKS_PER_ROW; ++column)
		{
			switch (row)
			{
				case 0:
				case 1:
				{
					brick->rect.colour = COLOUR_YELLOW;
					brick->value = 1;
				} break;

				case 2:
				case 3:
				{
					brick->rect.colour = COLOUR_GREEN;
					brick->value = 3;
				} break;

				case 4:
				case 5:
				{
					brick->rect.colour = COLOUR_ORANGE;
					brick->value = 5;
				} break;

				case 6:
				case 7:
				{
					brick->rect.colour = COLOUR_RED;
					brick->value = 7;
				} break;

				default:
				{
					brick->is_alive = false;
				} break;
			}

			brick->rect.width = BRICK_WIDTH;
			brick->rect.height = BRICK_HEIGHT;
			brick->rect.position.x = column * brick_spacing_x;
			brick->rect.position.y = y_offset + (row * brick_spacing_y);
			brick->is_alive = true;
			brick->padding = BRICK_PADDING;

			++brick;
		}
	}
}

void make_player(void)
{
	game.player.rect.colour = COLOUR_BLUE;
	game.player.rect.width = PLAYER_WIDTH;
	game.player.rect.height = PLAYER_HEIGHT;
	game.player.rect.position.x = (frame.width / 2) - (game.player.rect.width / 2);
	game.player.rect.position.y = game.player.rect.height;
}

void render_rect_to_frame(const RenderRect *rect)
{
	for (int y = 0; y < rect->height; ++y)
	{
		for (int x = 0; x < rect->width; ++x)
		{
			Pixel *pixel = frame.pixels + (frame.width * (rect->position.y + y)); // start of the rect in the y + current row of the rect to render
			pixel += rect->position.x + x;
			*pixel = rect->colour;
		}
	}
}

void render(void)
{
	memset(frame.pixels, 0, frame.width * frame.height * sizeof(Pixel));

	Brick *brick = game.bricks;
	
	for (int brick_index = 0; brick_index < TOTAL_BRICKS; ++brick_index)
	{
		if (brick->is_alive)
		{
			render_rect_to_frame(&brick->rect);	
		}		
			
		++brick;
	}
	
	render_rect_to_frame(&game.player.rect);
	
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

void update(void)
{
	
}