cmake_minimum_required(VERSION 3.5)
project(FileManager)

# ЖЕСТКО указываем консоль
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-subsystem,console")

set(PrName ${PROJECT_NAME})
set(MAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17) #версия с++
set(CMAKE_CXX_STANDARD_REQUIRED ON) #значит обязательно использовать указанный стандарт

add_executable(${PrName} src/main.cpp src/FileManager.cpp src/FileManagerUI.cpp src/FileManagerBK.cpp libs/sqlite3.c)

target_include_directories(${PrName} PRIVATE src libs)
