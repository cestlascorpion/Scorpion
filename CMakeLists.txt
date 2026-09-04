cmake_minimum_required(VERSION 3.16)

project(Scorpion LANGUAGES CXX)

find_package(Threads REQUIRED)

set(PROJECT_WARNINGS
    -Wall
    -Wextra
    -Wconversion
    -Wsizeof-pointer-memaccess
    -Wfloat-equal
    -Wconversion-null
    -Woverflow
    -Wshadow
    -faligned-new)
set(SCORPION_INCLUDE_DIRS
    src/algorithm
    src/basement
    src/concurrent
    src/encoding
    src/experiment
    src/format
    src/network
    src/utilities)

file(GLOB_RECURSE SCORPION_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE SCORPION_HEADERS CONFIGURE_DEPENDS src/*.h)

add_library(Scorpion STATIC ${SCORPION_SOURCES} ${SCORPION_HEADERS})
set_target_properties(Scorpion PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(Scorpion PRIVATE _FILE_OFFSET_BITS=64 _GNU_SOURCE)
target_compile_features(Scorpion PUBLIC cxx_std_17)
target_compile_options(Scorpion PRIVATE ${PROJECT_WARNINGS} -g -O0)
target_include_directories(Scorpion PUBLIC ${SCORPION_INCLUDE_DIRS})
target_link_libraries(Scorpion PUBLIC Threads::Threads)

foreach(target
    AsyncTaskPool
    BlockingQueue
    CMDStats
    ConsistentHash
    DigraphDot
    Encoding
    Formater
    IPV4Filter
    LeakyBucket
    LockFreeQueue
    NetHelper
    NRWLock
    SignalWrangler
    SpinLockMutex
    ThreadPool
    TimeWheel
    TokenBucket
    UnixSocket)
    add_executable(${target} test/${target}.cpp)
    target_compile_options(${target} PRIVATE ${PROJECT_WARNINGS} -g -O0)
    target_include_directories(${target} PRIVATE test)
    target_link_libraries(${target} PRIVATE Scorpion)
endforeach()
