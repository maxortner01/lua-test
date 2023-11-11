#pragma once

namespace S2D::Util
{

template<typename T>
struct Singleton
{
    inline static T& get();
    inline static void destroy();

private:
    inline static T* instance = nullptr;
};

template<typename T>
T& Singleton<T>::get()
{
    if (!instance) instance = new T();
    return *instance;
}

template<typename T>
void Singleton<T>::destroy()
{
    if (instance) delete instance;
    instance = nullptr;
}

} // S2D::Util