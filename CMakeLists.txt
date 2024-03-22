cmake_minimum_required(VERSION 3.22)
project(main VERSION 3.0 LANGUAGES C)

set(CMAKE_SIZEOF_VOID_P 4)

include(GNUInstallDirs)

add_subdirectory(vendor/nxp)
