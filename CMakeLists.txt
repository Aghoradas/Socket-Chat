cmake_minimum_required(VERSION 3.5)
project(wxWidgetsApp)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)

include(${wxWidgets_USE_FILE})

add_executable(wxWidgetsApp src/main.cpp)

target_link_libraries(wxWidgetsApp ${wxWidgets_LIBRARIES})

target_include_directories(wxWidgetsApp PRIVATE ${wxWidgets_INCLUDE_DIRS})

add_definitions(${wxWidgets_DEFINITIONS})

