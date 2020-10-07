#pragma once

#include <cstring>
#include <cstdint>

static const int INVALID_FD = -1;


#if defined(WIN32) || defined(__MINGW32__)

#else

typedef int					SOCKET;

#endif


#ifdef _MSC_VER
// Assuming windows is always little-endian.
#if _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)
// If MSVC has "/RTCc" set, it will complain about truncating casts at
// runtime.  This file contains some intentional truncating casts.
#pragma runtime_checks("c", off)
#endif
#else
#include <sys/param.h>  // __BYTE_ORDER
#if ((defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) ||    \
     (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN))
#define BEXMQ_LITTLE_ENDIAN 1
#endif
#endif


template <int>
struct EndianHelper;

template <>
struct EndianHelper<1>
{
    template<class T>
    static void read(const char* p, T& value) { value = *static_cast<const T*>(p); }

    template<class T>
    static void write(char* p, T value) { *static_cast<T*>(p) = value; }
};

template <>
struct EndianHelper<2>
{
    template<class T>
    static void read(const char* p, T& value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(&value, p, sizeof(T));
#else
        std::uint16_t v;
        std::memcpy(&v, p, sizeof(T));
        v = bswap_16(v);
        value = *reinterpret_cast<T*>(&v));
#endif
    }

    template<class T>
    static void write(char* p, T value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(p, &value, sizeof(T));
#else
        std::uint16_t v = *reinterpret_cast<T*>(&value));
        v = bswap_16(value);
        std::memcpy(p, &v, sizeof(T));
#endif
    }
};

template <>
struct EndianHelper<4> {
    template<class T>
    static void read(const char* p, T& value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(&value, p, sizeof(T));
#else
        std::uint32_t v;
        std::memcpy(&v, p, sizeof(T));
        v = bswap_32(v);
        value = *reinterpret_cast<T*>(&v));
#endif
    }

    template<class T>
    static void write(char* p, T value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(p, &value, sizeof(T));
#else
        std::uint32_t v = *reinterpret_cast<T*>(&value));
        v = bswap_32(value);
        std::memcpy(p, &v, sizeof(T));
#endif
    }
};

template <>
struct EndianHelper<8> {
    template<class T>
    static void read(const char* p, T& value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(&value, p, sizeof(T));
#else
        std::uint64_t v;
        std::memcpy(&v, p, sizeof(T));
        v = bswap_64(v);
        value = *reinterpret_cast<T*>(&v));
#endif
    }

    template<class T>
    static void write(char* p, T value)
    {
#ifdef BEXMQ_LITTLE_ENDIAN
        std::memcpy(p, &value, sizeof(T));
#else
        std::uint64_t v = *reinterpret_cast<T*>(&value));
        v = bswap_64(value);
        std::memcpy(p, &v, sizeof(T));
#endif
    }
};



namespace bex {

// constants
const unsigned int LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
const unsigned int SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;

// first 16bit in the lower word, second 16bit in higher word.
static unsigned int utf32to16(unsigned int utf32)
{
    if (utf32 > 0x0000ffff)
    {
        unsigned int lead = LEAD_OFFSET + (utf32 >> 10);
        unsigned int trail = 0xDC00 + (utf32 & 0x3FF);
        return (trail << 16) + lead;
    }
    else
    {
        return utf32;
    }
}

                                        // first 16bit in the lower word, second 16bit in higher word.
static unsigned int utf16to32(unsigned int utf16)
{
    unsigned int trail = utf16 >> 16;
    if (trail != 0)
    {
        unsigned int lead = utf16 & 0x0000ffff;
        return (lead << 10) + trail + SURROGATE_OFFSET;
    }
    else
    {
        return utf16;
    }
}
static unsigned int utf32to8(unsigned int utf32)
{
    if (utf32 <= 0x7f)
    {
        return utf32;
    }
    else if ((utf32 > 0x7f) && (utf32 <= 0x07ff))
    {
        unsigned int lead = (utf32 >> 6) | 0xc0;
        unsigned int trail = (utf32 & 0x3f) | 0x80;
        return (trail << 8) + lead;
    }
    else if ((utf32 > 0x07ff) && (utf32 <= 0xffff))
    {
        unsigned int lead = (utf32 >> 12) | 0xe0;
        unsigned int trail1 = ((utf32 >> 6) & 0x3f) | 0x80;
        unsigned int trail2 = (utf32 & 0x3f) | 0x80;
        return (trail2 << 16) + (trail1 << 8) + lead;
    }
    else
    {
        unsigned int lead = (utf32 >> 18) | 0xf0;
        unsigned int trail1 = ((utf32 >> 6) & 0x3f) | 0x80;
        unsigned int trail2 = ((utf32 >> 12) & 0x3f) | 0x80;
        unsigned int trail3 = (utf32 & 0x3f) | 0x80;
        return (trail3 << 24) + (trail2 << 16) + (trail1 << 8) + lead;
    }
}
static unsigned int utf8to32(unsigned int utf8)
{
    unsigned char c = utf8 & 0x000000ff;
    if (c <= 0x7f)
    {
        return c;
    }
    else if ((c >= 0xc0) && (c <= 0xdf))
    {
        unsigned char c2 = (utf8 >> 8) & 0x000000ff;
        return (((c & 0x1f) << 6) | (c2 & 0x3f));
    }
    else if ((c >= 0xe0) && (c <= 0xef))
    {
        unsigned char c2 = (utf8 >> 8)  & 0x000000ff;
        unsigned char c3 = (utf8 >> 16) & 0x000000ff;
        return (((c & 0x0f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f));
    }
    else
    {
        unsigned char c2 = (utf8 >> 8)  & 0x000000ff;
        unsigned char c3 = (utf8 >> 16) & 0x000000ff;
        unsigned char c4 = (utf8 >> 24) & 0x000000ff;
        return (((c & 0x07) << 18) | ((c2 & 0x3f) << 12) | ((c3 & 0x3f) << 6) | (c4 & 0x3f));
    }
}

}   // namespace
