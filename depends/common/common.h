#pragma once

//TYPES
typedef signed char         int8;
typedef unsigned char	    uint8;
typedef __int16             int16;
typedef unsigned __int16    uint16;
typedef __int32             int32;
typedef unsigned __int32    uint32;

typedef __int64             int64;
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
