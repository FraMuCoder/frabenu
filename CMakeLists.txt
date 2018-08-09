# frabenu - Framebuffer menu
# Copyright (C) 2018 Frank Müller
#
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 2.4)
project (frabenu)

add_compile_options("-Wall")

# for some FindXxx.cmake-files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# sources without main and lib dependend sources
set(FRABENU_BASE_SRC
    debug.c
    debug.h
    input.c
    input.h
    input_kbd.c
    input_kbd.h
    input_joy.c
    input_joy.h
    menu.c
    menu.h
    timer.c
    timer.h
    fbida/fb-gui.c
    fbida/fb-gui.h
    fbida/fbi.c
    fbida/fbi.h
    fbida/fbtools.c
    fbida/fbtools.h
    fbida/gfx.h
    fbida/kbd.c
    fbida/kbd.h
    fbida/list.h
    fbida/misc.h
    fbida/readers.c
    fbida/readers.h
    fbida/vt.c
    fbida/vt.h
    fbida/rd/read-bmp.c
    fbida/rd/read-ppm.c
)

# Search and add realy needed libs

find_package(Pixman REQUIRED)
include_directories(${PIXMAN_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PIXMAN_LIBRARIES})

# Search for extra libs and add them and extra sources

find_package(JPEG)
find_package(EXIF)
if (JPEG_FOUND AND EXIF_FOUND)
    include_directories(${JPEG_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${JPEG_LIBRARIES})
    execute_process(COMMAND "${CMAKE_SOURCE_DIR}/fbida/scripts/jpeg-version.sh"
                    OUTPUT_VARIABLE JPEG_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    include_directories(${CMAKE_SOURCE_DIR}/fbida/jpeg/${JPEG_VERSION})
    include_directories(${EXIF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${EXIF_LIBRARIES})
    set(FRABENU_BASE_SRC ${FRABENU_BASE_SRC}
        fbida/rd/read-jpeg.c
        fbida/jpeg/${JPEG_VERSION}/jinclude.h
        fbida/jpeg/${JPEG_VERSION}/jpegint.h
        fbida/jpeg/${JPEG_VERSION}/jpeglib.h
        fbida/jpeg/${JPEG_VERSION}/transupp.c
        fbida/jpeg/${JPEG_VERSION}/transupp.h)
endif (JPEG_FOUND AND EXIF_FOUND)

find_package(PNG)
if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${PNG_LIBRARIES})
    set(FRABENU_BASE_SRC ${FRABENU_BASE_SRC} fbida/rd/read-png.c)
endif (PNG_FOUND)

find_package(GIF)
if (GIF_FOUND)
    include_directories(${GIF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${GIF_LIBRARIES})
    set(FRABENU_BASE_SRC ${FRABENU_BASE_SRC} fbida/rd/read-gif.c)
endif (GIF_FOUND)

find_package(TIFF)
if (TIFF_FOUND)
    include_directories(${TIFF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${TIFF_LIBRARIES})
    set(FRABENU_BASE_SRC ${FRABENU_BASE_SRC} fbida/rd/read-tiff.c)
endif (TIFF_FOUND)

# The main application

add_executable(frabenu "frabenu.c" ${FRABENU_BASE_SRC})
set_property(TARGET frabenu PROPERTY C_STANDARD 11)
target_link_libraries(frabenu ${LIBS})

# Some simple test code

enable_testing()

add_executable(frabenu_test "test/test.c" ${FRABENU_BASE_SRC})
target_link_libraries(frabenu_test ${LIBS})
add_test(NAME FrabenuTest COMMAND frabenu_test WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/example")
