project(aprof C)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
enable_language(ASM-VASM)

add_executable(aprof
    src/dsp.c
    src/freedosobject.c
    src/p3amigaguide.c
    src/p3break.c
    src/p3exec.c
    src/p3funcs.c
    src/p3gui.c
    src/p3load.c
    src/p3main.c
    src/p3menu.c
    src/p3prefs.c
    src/p3rdsym.c
    src/p3rexx.c
    src/p3search.c
    src/p3symdis.c
    src/p3text.c
    src/p3timer.c
    src/p3trap.s
    src/p3traphandler.c
    src/p3xseglist.s
)