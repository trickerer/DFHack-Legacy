#include "common.h"
#include <assert.h>
#include <iostream>

#ifdef  __cplusplus
extern "C" {
_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
}
#else
_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
#endif

#undef Assert_Type
#define Assert_Type(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )

namespace CountedTypes {

void checkString24() {
    if (!(sizeof(std::string24) == 24)) {
        std::cerr << "\nType assertion fail: string24";
        Assert_Type(false);
    }
}
void checkVector12() {
    if (!(sizeof(std::vector12<int>) == 12)) {
        std::cerr << "\nType assertion fail: vector12<int>";
        Assert_Type(false);
    }
}
void checkVector12Bool() {
    if (!(sizeof(std::vector12<bool>) == 16)) {
        std::cerr << "\nType assertion fail: vector12<bool>";
        Assert_Type(false);
    }
}
void checkDeque20() {
    if (!(sizeof(std::deque20<int>) == 20)) {
        std::cerr << "\nType assertion fail: std::deque20<int>";
        Assert_Type(false);
    }
}
void checkFstream() {
    if (!(sizeof(fstream_empty) == 144)) {
        std::cerr << "\nType assertion fail: fstream_empty";
        Assert_Type(false);
    }
}

void CheckTypes()
{
    checkString24();
    checkVector12();
    checkVector12Bool();
    checkDeque20();
    checkFstream();
}
}
