set(GCC_VERSION ${version})
set(CMAKE_SYSTEM_NAME ${triple}_BARE_METAL)

# Set paths for various compiler tools using the COMPILER_PATH
if(WIN32)
    set(CMAKE_ADDR2LINE ${COMPILER_PATH}/bin/${triple}-addr2line.exe)
    set(CMAKE_AR ${COMPILER_PATH}/bin/${triple}-ar.exe)
    set(CMAKE_RANLIB ${COMPILER_PATH}/bin/${triple}-ranlib.exe)
    set(CMAKE_NM ${COMPILER_PATH}/bin/${triple}-nm.exe)
    set(CMAKE_OBJCOPY ${COMPILER_PATH}/bin/${triple}-objcopy.exe)
    set(CMAKE_OBJDUMP ${COMPILER_PATH}/bin/${triple}-objdump.exe)
    set(CMAKE_READELF ${COMPILER_PATH}/bin/${triple}-readelf.exe)
    set(CMAKE_STRIP ${COMPILER_PATH}/bin/${triple}-strip.exe)
    set(CMAKE_ASM_COMPILER ${COMPILER_PATH}/bin/${triple}-gcc.exe)
    set(CMAKE_C_COMPILER ${COMPILER_PATH}/bin/${triple}-gcc.exe)
    set(CMAKE_C_COMPILER_AR ${COMPILER_PATH}/bin/${triple}-ar.exe)
    set(CMAKE_C_COMPILER_RANLIB ${COMPILER_PATH}/bin/${triple}-ranlib.exe)
    set(CMAKE_CXX_COMPILER ${COMPILER_PATH}/bin/${triple}-g++.exe)
    set(CMAKE_LINKER ${COMPILER_PATH}/bin/${triple}-ld.exe)
else()
    set(CMAKE_ADDR2LINE ${COMPILER_PATH}/bin/${triple}-addr2line)
    set(CMAKE_AR ${COMPILER_PATH}/bin/${triple}-ar)
    set(CMAKE_RANLIB ${COMPILER_PATH}/bin/${triple}-ranlib)
    set(CMAKE_NM ${COMPILER_PATH}/bin/${triple}-nm)
    set(CMAKE_OBJCOPY ${COMPILER_PATH}/bin/${triple}-objcopy)
    set(CMAKE_OBJDUMP ${COMPILER_PATH}/bin/${triple}-objdump)
    set(CMAKE_READELF ${COMPILER_PATH}/bin/${triple}-readelf)
    set(CMAKE_STRIP ${COMPILER_PATH}/bin/${triple}-strip)
    set(CMAKE_ASM_COMPILER ${COMPILER_PATH}/bin/${triple}-gcc)
    set(CMAKE_C_COMPILER ${COMPILER_PATH}/bin/${triple}-gcc)
    set(CMAKE_C_COMPILER_AR ${COMPILER_PATH}/bin/${triple}-ar)
    set(CMAKE_C_COMPILER_RANLIB ${COMPILER_PATH}/bin/${triple}-ranlib)
    set(CMAKE_CXX_COMPILER ${COMPILER_PATH}/bin/${triple}-g++)
    set(CMAKE_LINKER ${COMPILER_PATH}/bin/${triple}-ld)
endif()

set(CPU_SETTINGS "${GCC_FLAGS}")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -ggdb")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -flto -ffat-lto-objects")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_SETTINGS}")

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
	    ${COMPILER_PATH}/lib/gcc/${triple}/${GCC_VERSION}/include
        ${COMPILER_PATH}/lib/gcc/${triple}/${GCC_VERSION}/include-fixed)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles -nostdlib -nostdinc -ffreestanding")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Wno-builtin-declaration-mismatch")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_SETTINGS} -std=gnu2x")

set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
	    ${COMPILER_PATH}/lib/gcc/${triple}/${GCC_VERSION}/include
        ${COMPILER_PATH}/lib/gcc/${triple}/${GCC_VERSION}/include-fixed)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ffat-lto-objects")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostartfiles -nostdlib -nostdinc -ffreestanding -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wno-builtin-declaration-mismatch")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_SETTINGS} -std=gnu++20")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles -nostdlib -nostdinc -ffreestanding")