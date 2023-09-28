cmake_minimum_required(VERSION 3.0.0)
project(hello VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(hello
    hello.c
    cpuintrf.c
    cpuexec.c
    cpuint.c
    timer.c
    memory.c
    cpu/m6809/m6809.c
    cpu/m6809/6809dasm.c
    cpu/m6809/6809tbl.c
    cpu/m6809/6809ops.c
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
