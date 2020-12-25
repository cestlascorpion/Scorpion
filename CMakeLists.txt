cmake_minimum_required(VERSION 3.16)

project(Scorpion)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow -faligned-new \
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O3")

include_directories(src test)

add_executable(AsyncTaskPool
        src/MPMCQueue.h
        src/MPSCQueue.h
        src/AsyncTaskPool.h
        src/AsyncTaskPoolTemplate.h
        test/AsyncTaskPool.cpp)

add_executable(BlockingQueue
        src/BlockingQueue.h
        test/BlockingQueue.cpp)

add_executable(ConsistentHash
        src/ConsistentHash.h
        test/ConsistentHash.cpp)

add_executable(IPV4Filter
        src/IPV4Filter.h
        src/IPV4Filter.cpp
        test/IPV4Filter.cpp)

add_executable(LeakyBucket
        src/LeakyBucket.h
        test/LeakyBucket.cpp)

add_executable(LockFreeQueue
        src/MPMCQueue.h
        src/MPSCQueue.h
        src/SPSCQueue.h
        test/LockFreeQueue.cpp)

add_executable(NRWLock
        src/SingletonTemplate.h
        src/FLock.h
        src/FLock.cpp
        src/NRWLock.h
        src/NRWLock.cpp
        test/NRWLock.cpp)

add_executable(SpinLockMutex
        src/SpinLockMutex.h
        test/SpinLockMutex.cpp)

add_executable(TimeWheel
        src/TimeWheel.h
        src/TimeWheel.cpp
        test/TimeWheel.cpp)

add_executable(TokenBucket
        src/TokenBucket.h
        test/TokenBucket.cpp)

add_executable(CMDStats
        src/CMDStats.h
        src/CMDStats.cpp
        test/CMDStats.cpp)