#pragma once

#include <stdint.h>

#ifdef FIZZENGINE_EXPORTS
// When building the engine, export symbols
#define FIZZENGINE_API __declspec(dllexport)
#else
// When using the engine in another project, import symbols
#define FIZZENGINE_API __declspec(dllimport)
#endif

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#include <spdlog/spdlog.h>

// Native types typedefs /////////////////////////////////////////////////
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;

typedef size_t      sizet;

typedef const char* cstring;

static const u64    u64_max = UINT64_MAX;
static const i64    i64_max = INT64_MAX;
static const u32    u32_max = UINT32_MAX;
static const i32    i32_max = INT32_MAX;
static const u16    u16_max = UINT16_MAX;
static const i16    i16_max = INT16_MAX;
static const u8     u8_max  = UINT8_MAX;
static const i8     i8_max  = INT8_MAX;
