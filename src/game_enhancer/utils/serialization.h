#pragma once

#include <iostream>

namespace GE
{
    class BinWriter
    {
        std::ostream* m_out;

    public:
        // Non-explicit to make seamless transition from std::ostream
        BinWriter(std::ostream& out)
            : m_out(&out)
        {
        }

        template <class T>
        auto& Write(const T& val)
        {
            m_out->write(reinterpret_cast<const char*>(&val), sizeof(val));
            return *this;
        }

        template <class T>
        auto& Write(const T* val, size_t size = sizeof(T))
        {
            m_out->write(reinterpret_cast<const char*>(val), size);
            return *this;
        }
    };

    class BinReader
    {
        std::istream* m_in;

    public:
        // Non-explicit to make seamless transition from std::istream
        BinReader(std::istream& in)
            : m_in(&in)
        {
        }

        template <class T>
        auto& Read(T& val)
        {
            m_in->read(reinterpret_cast<char*>(&val), sizeof(val));
            return *this;
        }

        template <class T>
        auto& Read(T* val, size_t size = sizeof(T))
        {
            m_in->read(reinterpret_cast<char*>(val), size);
            return *this;
        }

        template <class T>
        [[nodiscard]] T Read()
        {
            T t{};
            Read(t);
            return t;
        }
    };
}
