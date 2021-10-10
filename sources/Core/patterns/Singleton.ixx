export module Singleton;

import stl.memory;
import stl.core;
import stl.threading;

// Use this class only if you really need shared_ptr of singleton's instance.
// "With Great Power Comes Great Responsibility" (c)
export template <class T>
 class SingletonAccessor
{
    public:
        static std::shared_ptr<T> get_native();
};

export template <typename T>
 class Singleton
{
        static T* ptr;
        static std::vector<char> memory;
        static std::shared_ptr<T> instance;

        static std::mutex create_mutex;
        friend class SingletonAccessor<T>;

    protected:
        Singleton() = default;
        virtual  ~Singleton() = default;
    public:

        template<typename G = T>
        static T * create() requires(std::is_abstract_v<G>)
        {
            return instance.get();
        }

        template<typename G = T>
        static T * create() requires(!std::is_abstract_v<G>)
        {
            if (ptr) return ptr;

            if (!instance)
            {
                std::lock_guard<std::mutex> g(create_mutex);

                if (instance) return instance.get();

                memory.resize(sizeof(G));
              auto new_ptr = reinterpret_cast<T*>(memory.data());
                // auto stack = get_stack_trace();
                instance = std::shared_ptr<T>(new(new_ptr)G(), [](T * p)
                {
                    if (!p)
                        return;

                    p->~T();
                    memory.clear();
                    p = nullptr;
                    ptr = nullptr;
                });

				ptr = new_ptr;
            }

            return instance.get();
        }


        static void reset()
        {
            if (memory.size())
                instance = nullptr;
        }

        template<typename G = T>
        static T & get()
        {
            return *create<G>();
        }

        static bool is_good()
        {
            return instance.get();
        }
};

module: private;

template <typename T>
std::shared_ptr<T>  Singleton<T>::instance;
template <typename T>
T*  Singleton<T>::ptr = nullptr;

template <typename T>
std::vector<char> Singleton<T>::memory;
template <typename T>
std::mutex Singleton<T>::create_mutex;

template <class T>
std::shared_ptr<T> SingletonAccessor<T>::get_native()
{
    return Singleton<T>::instance;
}