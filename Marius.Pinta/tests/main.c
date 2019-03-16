#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "pinta_tests.h"

char message[400];
char *pinta_tests_message(const wchar *format, ...)
{
    wchar buffer[200];

    va_list list;

    va_start(list, format);
    vswprintf(buffer, 200, format, list);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

    return message;
}

int main()
{
    char gets_buffer[1024];
    pinta_tests();
    gets_s(gets_buffer, 1024);

    return 0;
}
