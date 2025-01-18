#include "stdafx.h"

#include "command_queue.h"

#include "engine/renderer/device.h"
#include "engine/renderer/dx12_lib.h"

namespace engine
{
  fcommand_queue::fcommand_queue(fdevice* device, uint32_t in_back_buffer_count)
  {
    back_buffer_count = in_back_buffer_count;
    device->create_command_queue(com);
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      fcommand_pair temp;
      device->create_command_list(back_buffer_count, temp.command_list.com, temp.command_allocator);
#if BUILD_DEBUG
      for(uint32_t n = 0; n < back_buffer_count; n++)
      {
        DX_SET_NAME(temp.command_allocator[n], "Command allocator: type {} back buffer count {}", i, n)
        DX_SET_NAME(temp.command_list.com, "Command list: type {}", i)
      }
#endif
      command_pairs.push_back(temp);
    }
    device->create_synchronisation(back_buffer_count, next_fence_value, fence, fence_event, fence_value);
  }

  fcommand_queue::~fcommand_queue()
  {
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      flush();
    }

    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      command_pairs[i].command_list.com->Close();
    }
    
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      reset_allocator(n);
    }
    
    CloseHandle(fence_event);
  }

  bool fcommand_queue::is_fence_complete(uint64_t value) const
  {
    return fence->GetCompletedValue() >= value;
  }

  void fcommand_queue::wait_for_fence_value(uint64_t value)
  {
    if(!is_fence_complete(value))
    {
      THROW_IF_FAILED(fence->SetEventOnCompletion(value, fence_event))
      WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    }
  }

  uint64_t fcommand_queue::signal()
  {
    THROW_IF_FAILED(com->Signal(fence.Get(), ++next_fence_value))
    return next_fence_value;
  }

  void fcommand_queue::flush()
  {
    LOG_INFO("GPU Flush!")
    wait_for_fence_value(signal());
  }

  fcommand_list* fcommand_queue::get_command_list(ecommand_list_purpose type, uint32_t back_buffer_id)
  {
    return &command_pairs[type].command_list;
  }

  void fcommand_queue::reset_allocator(uint32_t back_buffer_id) const
  {
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      command_pairs[i].command_allocator[back_buffer_id]->Reset();
    }    
  }

  uint64_t fcommand_queue::execute_command_lists(uint32_t back_buffer_id)
  {
    // Close
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      command_pairs[i].command_list.com->Close();
    }

    // Submit
    std::vector<ID3D12CommandList*> command_list_ptr;
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      ID3D12GraphicsCommandList* temp = command_pairs[i].command_list.com.Get();
      command_list_ptr.push_back(temp);
    }
    com->ExecuteCommandLists(ecommand_list_purpose::num, command_list_ptr.data());
    fence_value[back_buffer_id] = signal();

    // Reset
    for(int i = 0; i < ecommand_list_purpose::num; i++)
    {
      command_pairs[i].command_list.com->Reset(command_pairs[i].command_allocator[back_buffer_id].Get(), nullptr);
    }
    
    return fence_value[back_buffer_id];
  }
}