cmake_minimum_required(VERSION 3.16)

project(Scorpion)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow -faligned-new \
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O3")

include_directories(src/algorithm src/base src/concurrent src/experiment src/network src/utilities test)

add_executable(AsyncTaskPool
        src/concurrent/MPMCQueue.h
        src/concurrent/MPSCQueue.h
        src/utilities/AsyncTaskPool.h
        src/utilities/AsyncTaskPoolTemplate.h
        test/AsyncTaskPool.cpp)

add_executable(BlockingQueue
        src/concurrent/BlockingQueue.h
        test/BlockingQueue.cpp)

add_executable(ConsistentHash
        src/algorithm/ConsistentHash.h
        test/ConsistentHash.cpp)

add_executable(IPV4Filter
        src/utilities/IPV4Filter.h
        src/utilities/IPV4Filter.cpp
        test/IPV4Filter.cpp)

add_executable(LeakyBucket
        src/algorithm/LeakyBucket.h
        test/LeakyBucket.cpp)

add_executable(LockFreeQueue
        src/concurrent/MPMCQueue.h
        src/concurrent/MPSCQueue.h
        src/concurrent/SPSCQueue.h
        test/LockFreeQueue.cpp)

add_executable(NRWLock
        src/base/SingletonTemplate.h
        src/utilities/FLock.h
        src/utilities/FLock.cpp
        src/experiment/NRWLock.h
        src/experiment/NRWLock.cpp
        test/NRWLock.cpp)

add_executable(SpinLockMutex
        src/concurrent/SpinLockMutex.h
        test/SpinLockMutex.cpp)

add_executable(TimeWheel
        src/algorithm/TimeWheel.h
        src/algorithm/TimeWheel.cpp
        test/TimeWheel.cpp)

add_executable(TokenBucket
        src/algorithm/TokenBucket.h
        test/TokenBucket.cpp)

add_executable(CMDStats
        src/utilities/CMDStats.h
        src/utilities/CMDStats.cpp
        test/CMDStats.cpp)