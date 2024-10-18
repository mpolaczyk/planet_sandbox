#include "d3d12.h"
#include <iostream>
#include <format>

#include "command_queue.h"

#include "core/exceptions.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  void fcommand_queue::init(ComPtr<ID3D12Device> device, uint32_t in_back_buffer_count)
  {
    back_buffer_count = in_back_buffer_count;
    fdx12::create_command_queue(device, command_queue);
    for(int i = 0; i < ecommand_list_type::num; i++)
    {
      fcommand_pair temp;
      fdx12::create_command_list(device, back_buffer_count, temp.command_list, temp.command_allocator);
#if BUILD_DEBUG
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        std::string allocator_name = std::format("Command allocator: type {} back buffer count {}", i, n);
        std::string list_name = std::format("Command list: type {}", i);
        temp.command_allocator[n]->SetName(std::wstring(allocator_name.begin(), allocator_name.end()).c_str());
        temp.command_list->SetName(std::wstring(list_name.begin(), list_name.end()).c_str());
      }
#endif
      command_pair.push_back(temp);
    }
    fdx12::create_synchronisation(device, back_buffer_count, last_fence_value, fence, fence_event, fence_value);
  }

  void fcommand_queue::cleanup()
  {
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      reset_command_lists(n);
    }
    
    for(int i = 0; i < ecommand_list_type::num; i++)
    {
      fcommand_pair& pair = command_pair[i];
      DX_RELEASE(pair.command_list)
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        DX_RELEASE(pair.command_allocator[n])
      }      
    }
    DX_RELEASE(command_queue)
    DX_RELEASE(fence)
    CloseHandle(fence_event);
  }

  bool fcommand_queue::is_fence_complete(uint64_t value) const
  {
    return fence->GetCompletedValue() >= value;
  }

  void fcommand_queue::wait_for_fence_value(uint64_t value) const
  {
    if(!is_fence_complete(value))
    {
      THROW_IF_FAILED(fence->SetEventOnCompletion(value, fence_event))
      WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    }
  }

  uint64_t fcommand_queue::signal()
  {
    uint64_t value_for_signal = ++last_fence_value;
    THROW_IF_FAILED(command_queue->Signal(fence.Get(), value_for_signal))
    return value_for_signal;
  }

  void fcommand_queue::flush()
  {
    wait_for_fence_value(signal());
  }

  ComPtr<ID3D12CommandQueue> fcommand_queue::get_command_queue() const
  {
    return command_queue;
  }

  void fcommand_queue::close_command_lists(uint32_t back_buffer_id)
  {
    for(int i = 0; i < ecommand_list_type::num; i++)
    {
      fcommand_pair& pair = command_pair[i];
      pair.command_list->Close();
    }
  }
  
  void fcommand_queue::reset_command_lists(uint32_t back_buffer_id)
  {
    for(int i = 0; i < ecommand_list_type::num; i++)
    {
      fcommand_pair& pair = command_pair[i];
      pair.command_allocator[back_buffer_id]->Reset();
      pair.command_list->Reset(pair.command_allocator[back_buffer_id].Get(), nullptr);
    }
  }

  ComPtr<ID3D12GraphicsCommandList> fcommand_queue::get_command_list(ecommand_list_type type, uint32_t back_buffer_id) const
  {
    return command_pair[type].command_list;
  }

  uint64_t fcommand_queue::execute_command_lists(uint32_t back_buffer_id)
  {
    std::vector<ID3D12CommandList*> command_list_ptr;
    for(int i = 0; i < ecommand_list_type::num; i++)
    {
      ID3D12GraphicsCommandList* temp = command_pair[i].command_list.Get();
      temp->Close();
      command_list_ptr.push_back(temp);
    }
    command_queue->ExecuteCommandLists(ecommand_list_type::num, command_list_ptr.data());

    fence_value[back_buffer_id] = signal();
    return fence_value[back_buffer_id];
  }
}