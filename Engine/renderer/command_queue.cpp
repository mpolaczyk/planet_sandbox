#include "command_queue.h"

#include "core/exceptions.h"
#include "renderer/dx12_lib.h"

namespace engine
{
  void fcommand_queue::init(const ComPtr<ID3D12Device>& in_device, int num_backbuffers)
  {
    fdx12::create_command_queue(in_device, command_queue);
    fdx12::create_command_list(in_device, num_backbuffers, command_list, command_allocator);
    fdx12::create_synchronisation(in_device, num_backbuffers, last_fence_value, fence, fence_event, fence_value);
    back_buffer_count = num_backbuffers;
  }

  void fcommand_queue::cleanup()
  {
    for(uint32_t n = 0; n < back_buffer_count; n++)
    {
      command_allocator[n].Reset();
    }
    DX_RELEASE(command_queue);
    DX_RELEASE(command_list);
    DX_RELEASE(fence);
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
    THROW_IF_FAILED(command_queue->Signal(fence.Get(), value_for_signal));
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

  ComPtr<ID3D12GraphicsCommandList> fcommand_queue::get_command_list(int back_buffer_id) const
  {
    command_allocator[back_buffer_id]->Reset();
    command_list->Reset(command_allocator[back_buffer_id].Get(), nullptr);
    return command_list;
  }

  uint64_t fcommand_queue::execute_command_list(int back_buffer_id)
  {
    /// TODO: validate if value < number fo back buffers and if this buffer was requested in get_command_list
    command_list->Close();
    ID3D12CommandList* const command_lists[] = { command_list.Get() };
    command_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

    fence_value[back_buffer_id] = signal();
    return fence_value[back_buffer_id];
  }
}