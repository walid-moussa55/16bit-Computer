#include <iostream>
#include <cstdlib> // for std::malloc and std::free
#include <cstddef> // Required for std::byte


class Allocator{
public:
    Allocator(size_t bytes)
    : m_size(bytes)
    {
        std::cout<<"Creating Allocator..."<<std::endl;
        m_buffer = static_cast<std::byte*>(std::malloc(m_size));
        m_offset = m_buffer;
    }

    template<typename T>
    T* alloc(){
        if (m_offset + sizeof(T) > m_buffer + m_size) {
            throw std::bad_alloc();
        }
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }
    size_t usedSize() const {
        return static_cast<size_t>(m_offset - m_buffer);
    }

    Allocator(const Allocator& other) = delete;
    Allocator operator=(const Allocator& other) = delete;

    ~Allocator(){
        std::cout<<"Freeing Allocator..."<<std::endl;
        std::free(m_buffer);
    }

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};