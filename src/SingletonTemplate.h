/**
 * A singleton template.
 *
 */

#pragma once

namespace scorpion {

template <typename T>
class Singleton {
public:
    static T *Instance() {
        static T _inst;
        return &_inst;
    }

private:
    Singleton() = default;

public:
    virtual ~Singleton() = default;

    Singleton(const Singleton &) = delete;
    Singleton &operator=(Singleton &) = delete;
};

} // namespace scorpion
