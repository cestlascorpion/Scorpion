cmake_minimum_required(VERSION 3.16)
project(Scorpion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow -faligned-new\
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O0")

include_directories(
        src/algorithm
        src/basement
        src/concurrent
        src/experiment
        src/network
        src/utilities
        test)

file(GLOB_RECURSE SRCS src/*.cpp src/*/*.cpp)
file(GLOB_RECURSE HDRS src/*.h src/*/*.h)

add_library(Scorpion STATIC ${SRCS} ${HDRS})

foreach (_target
        AsyncTaskPool
        BlockingQueue
        CMDStats
        ConsistentHash
        IPV4Filter
        LeakyBucket
        LockFreeQueue
        NRWLock
        SpinLockMutex
        ThreadPool
        TimeWheel
        TokenBucket
        UnixSocket
        DigraphDot)
    add_executable(${_target} "test/${_target}.cpp")
    target_link_libraries(${_target} ${PROJECT_BINARY_DIR}/libScorpion.a)
endforeach ()