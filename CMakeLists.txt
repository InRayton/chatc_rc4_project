cmake_minimum_required(VERSION 3.30)
project(ChatApp)

set(CMAKE_CXX_STANDARD 20)

#минимизация включений winapi
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

#путь к wxWidgets
set(wxWidgets_ROOT_DIR "D:/wxWidgets-3.2.7")
set(wxWidgets_LIB_DIR "${wxWidgets_ROOT_DIR}/lib/gcc_lib")
set(wxWidgets_INCLUDE_DIRS
        "${wxWidgets_ROOT_DIR}/include"
        "${wxWidgets_LIB_DIR}/mswu"
)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

#собираем исполнительный файл
add_executable(ChatApp
        src/main.cpp
        src/RC4.cpp
        src/Messenger.cpp
)

target_link_libraries(ChatApp PRIVATE
        ${wxWidgets_LIBRARIES}
        uxtheme
        shell32
        comctl32
        rpcrt4
        oleacc
        oleaut32
        uuid
        winmm
        setupapi
        kernel32
        user32
        gdi32
        winspool
        comdlg32
        advapi32
)

target_include_directories(ChatApp PRIVATE
        ${wxWidgets_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
)
