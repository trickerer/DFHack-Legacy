#pragma once

#ifndef _COMMON_H_
#define _COMMON_H_

#define _SECURE_SCL 0   //do not inherit from secure base containers with extra debug allocations

#include "vector12.h"       //std::vector12<T>
#include "bvector12.h"      //std::vector12<bool>
#include "string24.h"       //std::string24
#include "deque20.h"        //std::deque20<T>

//TYPES
typedef signed __int8       int8;
typedef unsigned __int8     uint8;
typedef signed __int16      int16;
typedef unsigned __int16    uint16;
typedef signed __int32      int32;
typedef unsigned __int32    uint32;

typedef signed __int64      int64;
typedef unsigned __int64    uint64;

// compatibility helpers
typedef int8                int8_t;
typedef uint8               uint8_t;
typedef int16               int16_t;
typedef uint16              uint16_t;
typedef int32               int32_t;
typedef uint32              uint32_t;
typedef int64               int64_t;
typedef uint64              uint64_t;

// print helpers
#define INT64FMT "%I64d"
#define UINT64FMT "%I64u"

// placeholders
struct fstream_empty
{
    char empty[144];
};

namespace CountedTypes {
void CheckTypes();
}

#endif // _COMMON_H_
