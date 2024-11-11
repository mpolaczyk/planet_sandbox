#include "d3d12.h"
#include <iostream>
#include <format>

#include "command_queue.h"

#include "core/exceptions.h"
#include "renderer/device.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  void fcommand_queue::init(fdevice& device, uint32_t in_back_buffer_count)
  {
    back_buffer_count = in_back_buffer_count;
    device.create_command_queue(command_queue);
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      fcommand_pair temp;
      device.create_command_list(back_buffer_count, temp.command_list.com, temp.command_allocator);
#if BUILD_DEBUG
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        DX_SET_NAME(temp.command_allocator[n], "Command allocator: type {} back buffer count {}", i, n)
        DX_SET_NAME(temp.command_list.com, "Command list: type {}", i)
      }
#endif
      command_pair.push_back(temp);
    }
    device.create_synchronisation(back_buffer_count, last_fence_value, fence, fence_event, fence_value);
  }

  void fcommand_queue::cleanup()
  {
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      reset_command_lists(n);
    }
    
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      fcommand_pair& pair = command_pair[i];
      DX_RELEASE(pair.command_list.com)
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

  void fcommand_queue::close_command_lists()
  {
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      command_pair[i].command_list.com->Close();
    }
  }
  
  void fcommand_queue::reset_command_lists(uint32_t back_buffer_id)
  {
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      fcommand_pair& pair = command_pair[i];
      ID3D12CommandAllocator* command_allocator = pair.command_allocator[back_buffer_id].Get();
      command_allocator->Reset();
      pair.command_list.com->Reset(command_allocator, nullptr);
    }
  }

  std::shared_ptr<fgraphics_command_list> fcommand_queue::get_command_list(ecommand_list_purpose type, uint32_t back_buffer_id) const
  {
    return std::make_shared<fgraphics_command_list>(command_pair[type].command_list);
  }

  uint64_t fcommand_queue::execute_command_lists(uint32_t back_buffer_id)
  {
    std::vector<ID3D12CommandList*> command_list_ptr;
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      ID3D12GraphicsCommandList* temp = command_pair[i].command_list.com.Get();
      command_list_ptr.push_back(temp);
    }
    command_queue->ExecuteCommandLists(ecommand_list_purpose::num, command_list_ptr.data());

    fence_value[back_buffer_id] = signal();
    return fence_value[back_buffer_id];
  }
}