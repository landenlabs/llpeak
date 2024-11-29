//
//  Simple fixed size ring buffer.
//  Manage objects by value.
//  Thread safe for single Producer and single Consumer.
//


#pragma once
typedef unsigned int sizeT;

#include <assert.h>


template <class T, sizeT RingSize>
class RingBuffer
{
public:
    RingBuffer(sizeT size = 100)
        : m_size(size), m_buffer(new T[size]), m_rIndex(0), m_wIndex(0)
        { assert(size > 1 && m_buffer != nullptr); }

    ~RingBuffer()
        { delete [] m_buffer; };

    sizeT Next(sizeT n) const
        { return (n+1)%m_size; }
    bool Empty() const
        { return (m_rIndex == m_wIndex); }
    bool Full() const
        { return (Next(m_wIndex) == m_rIndex); }

    bool Put( T& value)
    {
        if (Full())
            return false;
        m_buffer[m_wIndex] = value;
        m_wIndex = Next(m_wIndex);
        return true;
    }

    bool Get(T& value)
    {
        if (Empty())
            return false;
        value = m_buffer[m_rIndex];
        m_rIndex = Next(m_rIndex);
        return true;
    }

private:
    T*             m_buffer;
    sizeT          m_size;

    // volatile is only used to keep compiler from placing values in registers.
    // volatile does NOT make the index thread safe.
    volatile sizeT m_rIndex;
    volatile sizeT m_wIndex;
};
