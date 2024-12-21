#pragma once

namespace engine
{
  template <typename T>
  struct funique_ptr final
  {
  private:
    T* ptr;

  public:
    explicit funique_ptr(T* ptr = nullptr) : ptr(ptr) {}

    ~funique_ptr() 
    {
      delete ptr;
    }

    funique_ptr(funique_ptr&& other) noexcept : ptr(other.ptr) 
    {
      other.ptr = nullptr;
    }

    funique_ptr& operator=(funique_ptr&& other) noexcept 
    {
      if (this != &other) 
      {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
      }
      return *this;
    }

    funique_ptr(const funique_ptr& other) = delete;

    funique_ptr& operator=(const funique_ptr& other) = delete;

    T& operator*() const 
    {
      return *ptr;
    }

    T* operator->() const 
    {
      return ptr;
    }

    T* get() const 
    {
      return ptr;
    }

    T* release() 
    {
      T* temp = ptr;
      ptr = nullptr;
      return temp;
    }

    void reset(T* newPtr = nullptr) 
    {
      delete ptr;
      ptr = newPtr;
    }

    bool operator!() const 
    {
      return ptr == nullptr;
    }

    explicit operator bool() const 
    {
      return ptr != nullptr;
    }
  };
}