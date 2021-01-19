/**
 * A simple task/thread pool template.
 * 
 */

#pragma once

namespace scorpion {

template <typename TASK, typename QUEUE>
class Worker {
public:
    Worker() = default;
    virtual ~Worker() = default;

public:
    // start the worker(eg: create the thread)
    virtual bool Start() = 0;
    // stop the worker(eg: stop and join the thread, clean the queue is necessary)
    virtual bool Stop(bool clean) = 0;
    // add a task into the queue
    virtual bool Add(TASK &&task) = 0;
};

template <typename TASK, typename QUEUE>
class Manager {
public:
    Manager() = default;
    virtual ~Manager() = default;

public:
    // create queues, workers and start workers
    virtual bool Init() = 0;
    // stop and clean the task pool
    virtual void Final(bool clean) = 0;
    // submit an async task
    virtual bool Submit(unsigned uid, TASK &&task) = 0;
};

} // namespace scorpion
