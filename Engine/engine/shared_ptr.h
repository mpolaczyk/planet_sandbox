#pragma once

namespace engine
{
  template <typename T>
  class fshared_ptr final
  {
  private:
    T* ptr;
    struct fcontrol_block 
    {
      int ref_count;
      fcontrol_block() : ref_count(1) { }
    }* control_block;

  public:
    explicit fshared_ptr(T* ptr = nullptr) 
      : ptr(ptr), control_block(nullptr) 
    {
      if (ptr) 
      {
        control_block = new fcontrol_block();
      }
    }

    ~fshared_ptr() 
    {
      release();
    }

    fshared_ptr(const fshared_ptr& other) 
      : ptr(other.ptr), control_block(other.control_block)
    {
      if (control_block)
      {
        ++control_block->ref_count;
      }
    }

    fshared_ptr& operator=(const fshared_ptr& other) 
    {
      if (this != &other) 
      {
        release();
        ptr = other.ptr;
        control_block = other.control_block;
        if (control_block) 
        {
          ++control_block->ref_count;
        }
      }
      return *this;
    }

    fshared_ptr(fshared_ptr&& other) noexcept 
      : ptr(other.ptr), control_block(other.control_block) 
    {
      other.ptr = nullptr;
      other.control_block = nullptr;
    }

    fshared_ptr& operator=(fshared_ptr&& other) noexcept 
    {
      if (this != &other) 
      {
        release();
        ptr = other.ptr;
        control_block = other.control_block;
        other.ptr = nullptr;
        other.control_block = nullptr;
      }
      return *this;
    }

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

    void reset(T* newPtr = nullptr) 
    {
      release();
      ptr = newPtr;
      if (ptr) 
      {
        control_block = new fcontrol_block();
      }
    }

    T* release() 
    {
      T* temp = ptr;
      if (control_block && --control_block->ref_count == 0)
      {
        delete ptr;
        delete control_block;
      }
      ptr = nullptr;
      control_block = nullptr;
      return temp;
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