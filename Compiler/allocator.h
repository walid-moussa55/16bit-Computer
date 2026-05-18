#pragma once
#include <iostream>
#include <cstdlib> // for std::malloc and std::free
#include <cstddef> // Required for std::byte
#include <functional>
#include <memory>


class Allocator{
public:
    Allocator(size_t bytes)
    : m_size(bytes)
    {
        std::cout<<"Creating Allocator"<<std::endl;
        m_buffer = static_cast<std::byte*>(std::malloc(m_size));
        m_offset = m_buffer;
    }

    template<typename T>
    T* alloc(){
        size_t space = m_size - static_cast<size_t>(m_offset - m_buffer);
        void* ptr = m_offset;
        if (!std::align(alignof(T), sizeof(T), ptr, space))
            throw std::bad_alloc();
        m_offset = static_cast<std::byte*>(ptr) + sizeof(T);

        T* obj = static_cast<T*>(ptr);
        m_destructors.push_back([obj]() {
            obj->~T();
        });
        return obj;
    }
    size_t usedSize() const {
        return static_cast<size_t>(m_offset - m_buffer);
    }

    Allocator(const Allocator& other) = delete;
    Allocator operator=(const Allocator& other) = delete;

    ~Allocator(){
        std::cout<<"Freeing Allocator"<<std::endl;
        for (auto it = m_destructors.rbegin(); it != m_destructors.rend(); ++it)
            (*it)();
        std::free(m_buffer);
    }

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
    std::vector<std::function<void()>> m_destructors;
};