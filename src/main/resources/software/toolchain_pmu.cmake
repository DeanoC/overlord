get_filename_component(IKUY_PATH ./programs_host/ REALPATH)
get_filename_component(COMPILER_PATH ${IKUY_PATH}/compilers/microblaze-none-elf REALPATH)

set(GCC_VERSION "10.2.0")
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_ADDR2LINE ${COMPILER_PATH}/bin/microblaze-none-elf-addr2line)
set(CMAKE_AR ${COMPILER_PATH}/bin/microblaze-none-elf-ar)
set(CMAKE_RANLIB ${COMPILER_PATH}/bin/microblaze-none-elf-ranlib)
set(CMAKE_NM ${COMPILER_PATH}/bin/microblaze-none-elf-nm)
set(CMAKE_OBJCOPY ${COMPILER_PATH}/bin/microblaze-none-elf-objcopy)
set(CMAKE_OBJDUMP ${COMPILER_PATH}/bin/microblaze-none-elf-objdump)
set(CMAKE_READELF ${COMPILER_PATH}/bin/microblaze-none-elf-readelf)
set(CMAKE_STRIP ${COMPILER_PATH}/bin/microblaze-none-elf-strip)
set(CMAKE_ASM_COMPILER ${COMPILER_PATH}/bin/microblaze-none-elf-gcc)
set(CMAKE_C_COMPILER ${COMPILER_PATH}/bin/microblaze-none-elf-gcc)
set(CMAKE_C_COMPILER_AR ${COMPILER_PATH}/bin/microblaze-none-elf-ar)
set(CMAKE_C_COMPILER_RANLIB ${COMPILER_PATH}/bin/microblaze-none-elf-ranlib)

set(CMAKE_LINKER ${COMPILER_PATH}/bin/microblaze-none-elf-ld)

set(CPU_SETTINGS "-mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v9.2 -mxl-soft-mul -Wl,--build-id")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -ggdb")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CPU_SETTINGS}")

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
		${COMPILER_PATH}/lib/gcc/microblaze-none-elf/${GCC_VERSION}/include
      	${COMPILER_PATH}/lib/gcc/microblaze-none-elf/${GCC_VERSION}/include-fixed)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles -nostdlib -nostdinc -ffreestanding")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wpedantic -Wno-builtin-declaration-mismatch")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_SETTINGS} -std=gnu2x")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-dT=${CMAKE_CURRENT_LIST_DIR}/empty-file.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles -nostdlib -nostdinc")
