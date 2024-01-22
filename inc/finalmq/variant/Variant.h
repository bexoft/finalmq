//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <memory>
#include <string>

#include <assert.h>

#include "IVariantValue.h"

namespace finalmq
{
//////////////////////

const static int VARTYPE_NONE = 0;

class SYMBOLEXP Variant
{
public:
    Variant();

    Variant(std::shared_ptr<IVariantValue> value);

    template<class T>
    Variant(T data)
        : m_value(std::make_shared<typename VariantValueTypeInfo<T>::VariantValueType>(std::move(data)))
    {
    }

    template<class T>
    const Variant& operator=(T data)
    {
        m_value = std::make_shared<typename VariantValueTypeInfo<T>::VariantValueType>(std::move(data));
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif
        return *this;
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
    }

    template<class T>
    operator T*()
    {
        if (m_value)
        {
            if (m_value->getType() == VariantValueTypeInfo<T>::VARTYPE)
            {
                return static_cast<T*>(m_value->getData());
            }
        }
        return nullptr;
    }

    template<class T>
    operator const T*() const
    {
        if (m_value)
        {
            if (m_value->getType() == VariantValueTypeInfo<T>::VARTYPE)
            {
                return static_cast<const T*>(m_value->getData());
            }
        }
        return nullptr;
    }

    template<class T>
    operator T() const
    {
        return VariantValueTypeInfo<T>::ConvertType::convert(*this);
    }

    template<class T>
    void convertVariantTo()
    {
        *this = VariantValueTypeInfo<T>::ConvertType::convert(*this);
    }

    template<class T>
    T* getData(const std::string& name)
    {
        Variant* variant = getVariant(name);
        if (variant)
        {
            std::shared_ptr<IVariantValue>& value = variant->m_value;
            if (value)
            {
                if (value->getType() == VariantValueTypeInfo<T>::VARTYPE)
                {
                    return static_cast<T*>(value->getData());
                }
            }
        }
        return nullptr;
    }

    template<class T>
    const T* getData(const std::string& name) const
    {
        return const_cast<Variant*>(this)->getData<T>(name);
    }

    template<class T>
    T getDataValue(const std::string& name) const
    {
        const Variant* variant = getVariant(name);
        if (variant)
        {
            return VariantValueTypeInfo<T>::ConvertType::convert(*variant);
        }
        return T();
    }

    inline int getType() const
    {
        if (m_value)
        {
            return m_value->getType();
        }
        else
        {
            return VARTYPE_NONE;
        }
    }

    Variant(const Variant& rhs);
    const Variant& operator=(const Variant& rhs);
    Variant(Variant&& rhs) noexcept;
    Variant& operator=(Variant&& rhs) noexcept;

    void accept(IVariantVisitor& visitor, ssize_t index = 0, int level = 0, ssize_t size = 0, const std::string& name = "");

    Variant* getVariant(const std::string& name);
    const Variant* getVariant(const std::string& name) const;

    bool operator==(const Variant& rhs) const;

    bool add(const std::string& name, const Variant& variant);
    bool add(const std::string& name, Variant&& variant);
    bool add(const Variant& variant);
    bool add(Variant&& variant);
    ssize_t size() const;

    Variant& getOrCreate(const std::string& name);

private:
    std::shared_ptr<IVariantValue> m_value{};
};

} // namespace finalmq

// includes, so that the application does not have to include these header files
#include "VariantValueList.h"
#include "VariantValueStruct.h"
#include "VariantValues.h"
