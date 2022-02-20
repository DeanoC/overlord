#pragma once
#include "core/core.h"

// platform options
#define IKUY_HOST_PLATFORM_WINDOWS    (0)
#define IKUY_HOST_PLATFORM_APPLE_MAC  (1)
#define IKUY_HOST_PLATFORM_IPHONE     (2)
#define IKUY_HOST_PLATFORM_UNIX       (3)
#define IKUY_HOST_PLATFORM_ANDROID    (4)
#define IKUY_HOST_PLATFORM_UNKNOWN		(5)

// OS options
#define IKUY_HOST_OS_CUSTOM         (0)
#define IKUY_HOST_OS_WINDOWS        (1)
#define IKUY_HOST_OS_OSX            (2)
#define IKUY_HOST_OS_GNULINUX       (3)
#define IKUY_HOST_OS_FREEBSD        (4)
#define IKUY_HOST_OS_ANDROID        (5)

// compiler family
#define IKUY_HOST_COMPILER_MSVC     (0)
#define IKUY_HOST_COMPILER_GCC      (1)
#define IKUY_HOST_COMPILER_CLANG    (2)
#define IKUY_HOST_COMPILER_CUDA     (3)

// endianess
#define IKUY_HOST_CPU_LITTLE_ENDIAN (0)
#define IKUY_HOST_CPU_BIG_ENDIAN    (1)

#define IKUY_HOST_CPU_X86           (0)
#define IKUY_HOST_CPU_X64           (1)
#define IKUY_HOST_CPU_ARM           (2)

//--------------------------------------------------------
// Identification and classification from Compiler Defines
// Most of this info is from http://predef.sourceforge.net
//--------------------------------------------------------

// Processor and endian-ness identification
#if defined( i386 ) || defined( __i386__ ) || defined( __i386 ) || \
    defined( _M_IX86 ) || defined( __X86__ ) || defined( _X86_ ) || \
    defined( __THW_INTEL__ ) || defined( __I86__ ) || defined( __INTEL__ )
#define IKUY_HOST_CPU_FAMILY    IKUY_HOST_CPU_X86
#define IKUY_HOST_CPU_ENDIANESS IKUY_HOST_CPU_LITTLE_ENDIAN
#define IKUY_HOST_CPU_BIT_SIZE  32
#elif defined( _M_X64 ) || defined( __amd64__ ) || defined( __amd64 ) || \
    defined( __x86_64__ ) || defined( __x86_64 )
#define IKUY_HOST_CPU_FAMILY    IKUY_HOST_CPU_X64
#define IKUY_HOST_CPU_ENDIANESS IKUY_HOST_CPU_LITTLE_ENDIAN
#define IKUY_HOST_CPU_BIT_SIZE  64

#elif defined(__aarch64__)

#define IKUY_HOST_CPU_FAMILY	IKUY_HOST_CPU_ARM
#if defined(__AARCH64EB__)
#define IKUY_HOST_CPU_ENDIANESS	IKUY_HOST_CPU_BIG_ENDIAN
#else
#define IKUY_HOST_CPU_ENDIANESS	IKUY_HOST_CPU_LITTLE_ENDIAN
#endif
#define IKUY_HOST_CPU_BIT_SIZE	64

#elif defined( __arm__ ) || defined( __thumb__ ) || defined( __TARGET_ARCH_ARM ) || defined( __TARGET_ARCH_THUMB ) || defined( _ARM )

#define IKUY_HOST_CPU_FAMILY   IKUY_HOST_CPU_ARM
#if defined(__ARMEB__) || defined(__THUMBEB__)
#define IKUY_HOST_CPU_ENDIANESS	IKUY_HOST_CPU_BIG_ENDIAN
#else
#define IKUY_HOST_CPU_ENDIANESS	IKUY_HOST_CPU_LITTLE_ENDIAN
#endif
#define IKUY_HOST_CPU_BIT_SIZE	32

#endif // end CPU Id


// compiler identifcation
#if defined( __clang__)

#define IKUY_HOST_COMPILER                  IKUY_HOST_COMPILER_CLANG
#include "compiler_clang.h"

#elif defined(__NVCC__)

#define AL203_COMPILER AL203_COMPILER_CUDA
#include "compiler_cuda.h"

#elif defined( _MSC_VER )

// compiler version used with above
#define IKUY_HOST_MS_VS2012                 (12)
#define IKUY_HOST_MS_VS2013                 (13)
#define IKUY_HOST_MS_VS2015                 (15)
#define IKUY_HOST_MS_VS2017                 (17)

// Minimum we support is VS 2012
#if _MSC_VER < 1800
#error Not supported
#endif // end _MSCVER < 1800

#define IKUY_HOST_COMPILER	IKUY_HOST_COMPILER_MSVC

#if _MSC_VER < 1800
#define IKUY_HOST_COMPILER_VERSION		IKUY_HOST_MS_VS2012
#elif _MSC_VER < 1900
#define IKUY_HOST_COMPILER_VERSION		IKUY_HOST_MS_VS2013
#elif _MSC_VER < 1910
#define IKUY_HOST_COMPILER_VERSION		IKUY_HOST_MS_VS2015
#else
#define IKUY_HOST_COMPILER_VERSION		IKUY_HOST_MS_VS2017
#endif

#include "compiler_msvc.h"


#elif defined( __GNUC__ )
#define IKUY_HOST_COMPILER				  IKUY_HOST_COMPILER_GCC
#define IKUY_HOST_GCC_V2                    (0)
#define IKUY_HOST_GCC_V3                    (1)
#define IKUY_HOST_GCC_V4                    (2)
#define IKUY_HOST_GCC_V4_3                  (3)

#include "compiler_gcc.h"

#else
#error Not supported
#endif

// OS
#if defined( WIN32 )

#	define IKUY_HOST_PLATFORM 				IKUY_HOST_PLATFORM_WINDOWS
#	define IKUY_HOST_PLATFORM_OS			IKUY_HOST_OS_WINDOWS

#include "platform_win.h"

#elif defined(__APPLE__) && defined( __MACH__ )

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#define IKUY_HOST_PLATFORM    IKUY_HOST_PLATFORM_IPHONE
#else
#define IKUY_HOST_PLATFORM    IKUY_HOST_PLATFORM_APPLE_MAC
#endif
#define IKUY_HOST_PLATFORM_OS IKUY_HOST_OS_OSX

// override endianness with the OS_OSX one, hopefully right...
#undef IKUY_HOST_CPU_ENDIANESS
#define IKUY_HOST_CPU_ENDIANESS (TARGET_RT_LITTLE_ENDIAN == 1)

#include "platform_osx.h"

#elif defined(__ANDROID__)

#define IKUY_HOST_PLATFORM      IKUY_HOST_PLATFORM_ANDROID
#define IKUY_HOST_PLATFORM_OS	IKUY_HOST_OS_ANDROID

#include "platform_android.h"

#elif    defined( __unix__ ) || defined( __unix ) || \
        defined( __sysv__ ) || defined( __SVR4 ) || defined( __svr4__ ) || defined( _SYSTYPE_SVR4 ) || \
        defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __bsdi__ ) || defined ( __DragonFly__ ) || defined( _SYSTYPE_BSD ) || \
        defined( sco ) || defined( _UNIXWARE7 ) || defined( ultrix ) || defined( __ultrix ) || defined( __ultrix__ ) || \
        defined( __osf__ ) || defined( __osf ) || defined( sun ) || defined( __sun ) || \
        defined( M_XENIX ) || defined( _SCO_DS ) || defined( sinux ) || defined( __minix ) || \
        defined( linux ) || defined( __linux ) || \
        defined( sgi ) || defined( __sgi ) || defined( __BEOS__ ) || defined (_AIX )

#define IKUY_HOST_PLATFORM IKUY_HOST_PLATFORM_UNIX

#if defined( linux ) || defined( __linux )

#define IKUY_HOST_PLATFORM_OS IKUY_HOST_OS_GNULINUX
#include "host_platform_linux.h"

#elif defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __bsdi__ ) || defined ( __DragonFly__ ) || defined( _SYSTYPE_BSD )

#define IKUY_HOST_PLATFORM_OS		IKUY_HOST_OS_FREEBSD
#include "platform_posix.h"

#else // unknown unix
#error Not supported
#endif

#else // unknown PLATFORM

#define IKUY_HOST_PLATFORM IKUY_HOST_PLATFORM_UNKNOWN

//#error UKNOWN_PLATFORM
#endif // endif OS
