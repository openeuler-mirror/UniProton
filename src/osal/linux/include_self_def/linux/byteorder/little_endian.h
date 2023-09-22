/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BYTEORDER_LITTLE_ENDIAN_H
#define _LINUX_BYTEORDER_LITTLE_ENDIAN_H

#include <linux/compiler_types.h>
#include <linux/types.h>

typedef unsigned short     __u16;
typedef unsigned int       __u32;
typedef unsigned long long __u64;

typedef __u16 __bitwise __le16;
typedef __u32 __bitwise __le32;
typedef __u64 __bitwise __le64;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __be64;

/*
 * casts are necessary for constants, because we never know how for sure
 * how U/UL/ULL map to __u16, __u32, __u64. At least not in a portable way.
 */
#define ___constant_swab16(x) ((__u16)(                \
    (((__u16)(x) & (__u16)0x00ffU) << 8) |            \
    (((__u16)(x) & (__u16)0xff00U) >> 8)))

#define ___constant_swab32(x) ((__u32)(                \
    (((__u32)(x) & (__u32)0x000000ffUL) << 24) |        \
    (((__u32)(x) & (__u32)0x0000ff00UL) <<  8) |        \
    (((__u32)(x) & (__u32)0x00ff0000UL) >>  8) |        \
    (((__u32)(x) & (__u32)0xff000000UL) >> 24)))

#define ___constant_swab64(x) ((__u64)(                \
    (((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) |    \
    (((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) |    \
    (((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) |    \
    (((__u64)(x) & (__u64)0x00000000ff000000ULL) <<  8) |    \
    (((__u64)(x) & (__u64)0x000000ff00000000ULL) >>  8) |    \
    (((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) |    \
    (((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) |    \
    (((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56)))

#define __swab16(x)         ___constant_swab16(x)
#define __swab32(x)         ___constant_swab32(x)
#define __swab64(x)         ___constant_swab64(x)

#define __cpu_to_le64(x) ((__force __le64)(__u64)(x))
#define __le64_to_cpu(x) ((__force __u64)(__le64)(x))
#define __cpu_to_le32(x) ((__force __le32)(__u32)(x))
#define __le32_to_cpu(x) ((__force __u32)(__le32)(x))
#define __cpu_to_le16(x) ((__force __le16)(__u16)(x))
#define __le16_to_cpu(x) ((__force __u16)(__le16)(x))
#define __cpu_to_be64(x) ((__force __be64)__swab64((x)))
#define __be64_to_cpu(x) __swab64((__force __u64)(__be64)(x))
#define __cpu_to_be32(x) ((__force __be32)__swab32((x)))
#define __be32_to_cpu(x) __swab32((__force __u32)(__be32)(x))
#define __cpu_to_be16(x) ((__force __be16)__swab16((x)))
#define __be16_to_cpu(x) __swab16((__force __u16)(__be16)(x))

static __always_inline __le64 __cpu_to_le64p(const __u64 *p)
{
    return (__force __le64)*p;
}
static __always_inline __u64 __le64_to_cpup(const __le64 *p)
{
    return (__force __u64)*p;
}
static __always_inline __le32 __cpu_to_le32p(const __u32 *p)
{
    return (__force __le32)*p;
}
static __always_inline __u32 __le32_to_cpup(const __le32 *p)
{
    return (__force __u32)*p;
}
static __always_inline __le16 __cpu_to_le16p(const __u16 *p)
{
    return (__force __le16)*p;
}
static __always_inline __u16 __le16_to_cpup(const __le16 *p)
{
    return (__force __u16)*p;
}

#endif /* _LINUX_BYTEORDER_LITTLE_ENDIAN_H */