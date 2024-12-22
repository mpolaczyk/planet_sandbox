#pragma once

#include "core/core.h"

#if !USE_CUSTOM_COM_POINTER

#include <wrl/client.h>

template<typename T>
using fcom_ptr = Microsoft::WRL::ComPtr<T>;

#else

namespace engine
{
  /* Lightweight equivalent of Microsoft::WRL::ComPtr 
  * T needs to be IUnknown type.
  * This saves on compile time significantly.
  * Shameless copy from wrl/client.h
  */
  template <typename T>
  class ENGINE_API fcom_ptr
  {
  public:
    typedef T InterfaceType;

protected:
    InterfaceType *ptr_;
    template<class U> friend class fcom_ptr;

    void InternalAddRef() const throw()
    {
        if (ptr_ != nullptr)
        {
            ptr_->AddRef();
        }
    }

    unsigned long InternalRelease() throw()
    {
        unsigned long ref = 0;
        T* temp = ptr_;

        if (temp != nullptr)
        {
            ptr_ = nullptr;
            ref = temp->Release();
        }

        return ref;
    }

public:
#pragma region constructors
    fcom_ptr() throw() : ptr_(nullptr)
    {
    }

    fcom_ptr(decltype(__nullptr)) throw() : ptr_(nullptr)
    {
    }

    template<class U>
    fcom_ptr(U *other) throw() : ptr_(other)
    {
        InternalAddRef();
    }

    fcom_ptr(const fcom_ptr& other) throw() : ptr_(other.ptr_)
    {
        InternalAddRef();
    }

    fcom_ptr(fcom_ptr &&other) throw() : ptr_(nullptr)
    {
        if (this != reinterpret_cast<fcom_ptr*>(&reinterpret_cast<unsigned char&>(other)))
        {
            Swap(other);
        }
    }
#pragma endregion

#pragma region destructor
    ~fcom_ptr() throw()
    {
        InternalRelease();
    }
#pragma endregion

#pragma region assignment
    fcom_ptr& operator=(decltype(__nullptr)) throw()
    {
        InternalRelease();
        return *this;
    }

    fcom_ptr& operator=(T *other) throw()
    {
        if (ptr_ != other)
        {
            fcom_ptr(other).Swap(*this);
        }
        return *this;
    }

    template <typename U>
    fcom_ptr& operator=(U *other) throw()
    {
        fcom_ptr(other).Swap(*this);
        return *this;
    }

    fcom_ptr& operator=(const fcom_ptr &other) throw()
    {
        if (ptr_ != other.ptr_)
        {
            fcom_ptr(other).Swap(*this);
        }
        return *this;
    }

    template<class U>
    fcom_ptr& operator=(const fcom_ptr<U>& other) throw()
    {
        fcom_ptr(other).Swap(*this);
        return *this;
    }

    fcom_ptr& operator=(fcom_ptr &&other) throw()
    {
        fcom_ptr(static_cast<fcom_ptr&&>(other)).Swap(*this);
        return *this;
    }

    template<class U>
    fcom_ptr& operator=(fcom_ptr<U>&& other) throw()
    {
        fcom_ptr(static_cast<fcom_ptr<U>&&>(other)).Swap(*this);
        return *this;
    }
#pragma endregion

#pragma region modifiers
    void Swap(fcom_ptr&& r) throw()
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }

    void Swap(fcom_ptr& r) throw()
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }
#pragma endregion

    operator bool() const throw() // planet620
    {
        return Get() != nullptr ? true : false; // planet620
    }

    T* Get() const throw()
    {
        return ptr_;
    }

    InterfaceType* operator->() const throw()
    {
        return ptr_;
    }

    T* const* GetAddressOf() const throw()
    {
        return &ptr_;
    }

    T** GetAddressOf() throw()
    {
        return &ptr_;
    }

    T** ReleaseAndGetAddressOf() throw()
    {
        InternalRelease();
        return &ptr_;
    }

    T* Detach() throw()
    {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void Attach(InterfaceType* other) throw()
    {
        if (ptr_ != nullptr)
        {
            auto ref = ptr_->Release();
            (void)ref;
        }

        ptr_ = other;
    }

    unsigned long Reset()
    {
        return InternalRelease();
    }
  };
}

#endif