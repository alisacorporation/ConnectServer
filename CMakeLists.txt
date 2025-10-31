# CMakeLists.txt for ConnectServer Cross-Platform Edition
cmake_minimum_required(VERSION 3.15)
project(ConnectServer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate build number (short hex like Git commits)
string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
string(MD5 BUILD_HASH "${BUILD_TIMESTAMP}")
string(SUBSTRING "${BUILD_HASH}" 0 7 BUILD_NUMBER)
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_TIME "%H:%M:%S")

message(STATUS "Build Number: ${BUILD_NUMBER}")
message(STATUS "Build Date: ${BUILD_DATE} ${BUILD_TIME}")
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")

# Options
option(USE_NCURSES "Use ncurses for terminal UI" OFF)
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# Find dependencies
find_package(Threads REQUIRED)

# Find Boost - try both methods
# Note: Boost.System is header-only since 1.69, Boost.Thread still needs linking
find_package(Boost 1.70 COMPONENTS thread)
if(NOT Boost_FOUND)
    message(STATUS "Trying CONFIG mode for Boost...")
    find_package(Boost 1.70 CONFIG COMPONENTS thread)
endif()

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please install via: vcpkg install boost-thread (or 'pacman -S boost' on Arch)")
endif()

if(USE_NCURSES)
    find_package(Curses REQUIRED)
    add_compile_definitions(USE_NCURSES)
endif()

# Source files (Phase 1 + Phase 2 + Phase 3)
set(SOURCES
    src/main.cpp
    src/ConfigManager.cpp
    src/CriticalSection.cpp
    src/Queue.cpp
    src/Console.cpp
    src/ConsoleInterface.cpp
    src/Util.cpp
    src/ClientSession.cpp
    src/SocketManager.cpp
    src/SocketManagerUdp.cpp
    src/TimerManager.cpp
    src/ReadScript.cpp
    src/IpManager.cpp
    src/ServerList.cpp
    src/ConnectServerProtocol.cpp
)

set(HEADERS
    include/ConfigManager.h
    include/CriticalSection.h
    include/Queue.h
    include/Console.h
    include/ConsoleInterface.h
    include/Util.h
    include/ProtocolDefines.h
    include/ClientSession.h
    include/SocketManager.h
    include/SocketManagerUdp.h
    include/TimerManager.h
    include/ReadScript.h
    include/IpManager.h
    include/ServerList.h
    include/ConnectServerProtocol.h
)

# Platform-specific sources
if(PLATFORM_WINDOWS)
    list(APPEND SOURCES src/platform/windows/CrashHandler.cpp)
    list(APPEND HEADERS include/platform/windows/CrashHandler.h)
elseif(PLATFORM_LINUX)
    list(APPEND SOURCES src/platform/linux/SignalHandler.cpp)
    list(APPEND HEADERS include/platform/linux/SignalHandler.h)
endif()

# Generate Version.h from template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    @ONLY
)
list(APPEND HEADERS ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h)

# Executable
add_executable(ConnectServer ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(ConnectServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Add Boost include directories (works for both old and new CMake configs)
if(Boost_INCLUDE_DIRS)
    target_include_directories(ConnectServer PRIVATE ${Boost_INCLUDE_DIRS})
elseif(TARGET Boost::headers)
    target_link_libraries(ConnectServer PRIVATE Boost::headers)
endif()

# Link libraries
target_link_libraries(ConnectServer
    PRIVATE
        Boost::thread
        Threads::Threads
)

# Link Boost::system only if it exists as a target (older Boost versions)
# In Boost 1.69+, system is header-only and doesn't need explicit linking
if(TARGET Boost::system)
    target_link_libraries(ConnectServer PRIVATE Boost::system)
endif()

if(USE_NCURSES)
    target_link_libraries(ConnectServer PRIVATE ${CURSES_LIBRARIES})
    target_include_directories(ConnectServer PRIVATE ${CURSES_INCLUDE_DIR})
endif()

# Platform-specific libraries
if(PLATFORM_WINDOWS)
    target_link_libraries(ConnectServer PRIVATE ws2_32 dbghelp)
elseif(PLATFORM_LINUX)
    target_link_libraries(ConnectServer PRIVATE dl)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ConnectServer PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-g -O0>
    )
    
    if(ENABLE_ASAN)
        target_compile_options(ConnectServer PRIVATE -fsanitize=address)
        target_link_options(ConnectServer PRIVATE -fsanitize=address)
    endif()
    
elseif(MSVC)
    target_compile_options(ConnectServer PRIVATE
        /W4
        /permissive-
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od /Zi>
    )
    
    # Enable multi-processor compilation
    target_compile_options(ConnectServer PRIVATE /MP)
endif()

# Install rules
install(TARGETS ConnectServer
    RUNTIME DESTINATION bin
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/config/ConnectServer.ini.example
    ${CMAKE_CURRENT_SOURCE_DIR}/config/ServerList.dat.example
    DESTINATION etc/connectserver
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/
    DESTINATION share/doc/connectserver
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Boost version: ${Boost_VERSION}")
message(STATUS "  Use ncurses: ${USE_NCURSES}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Enable ASAN: ${ENABLE_ASAN}")
message(STATUS "")
