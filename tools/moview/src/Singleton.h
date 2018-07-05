#pragma once

template <typename T>
class Singleton 
{
protected:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    virtual ~Singleton() = default;

public:
    static T& Instance()
    {
        static T _instance;
        return _instance;
    }
};
