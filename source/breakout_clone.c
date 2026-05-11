#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, char *command_line, int command_show)
{
    MessageBox(NULL, "Goodbye.", "Note", MB_OK);
    return 0;
}