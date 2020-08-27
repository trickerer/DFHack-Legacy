#include "common.h"
#include <assert.h>

#ifdef  __cplusplus
extern "C" {
_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
}
#else
_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);
#endif

#undef Assert_Type
#define Assert_Type(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )

void checkString24() {
    Assert_Type(sizeof(std::string) == 24);
}
void checkVector12() {
    Assert_Type(sizeof(std::vector12<int>) == 12);
}
void checkVector12Bool() {
    Assert_Type(sizeof(std::vector12<bool>) == 16);
}
void checkDeque20() {
    Assert_Type(sizeof(std::deque20<int>) == 20);
}
void checkFstream() {
    Assert_Type(sizeof(fstream_empty) == 144);
}

struct typeSizeChecker
{
    typeSizeChecker()
    {
        checkString24();
        checkVector12();
        checkVector12Bool();
        checkDeque20();
        checkFstream();
    }
}
CheckTypeSizes;
