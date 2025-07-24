# This file is used to set up the RISC-V toolchain for CMake.
# TO correctly activate the optimization flags use the following command when configuring the CMake before building:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=../riscv-toolchain.cmake -DCMAKE_BUILD_TYPE=Release

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)
## Specify the location of cross compiler in your system
set(CMAKE_C_COMPILER   "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-gcc")
set(CMAKE_CXX_COMPILER "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-g++")
set(CMAKE_AR           "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-ar")
set(CMAKE_RANLIB       "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-ranlib")
set(CMAKE_OBJCOPY      "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-objcopy")
set(CMAKE_OBJDUMP      "/home/omar/riscv-gnu-toolchain/build/install/bin/riscv64-unknown-elf-objdump")

set(CMAKE_C_FLAGS   "-march=rv64gcv_zvfh -mabi=lp64d" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-march=rv64gcv_zvfh -mabi=lp64d" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O2" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "" FORCE)
# Prevent CMake probing adds -latomic
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES   "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

