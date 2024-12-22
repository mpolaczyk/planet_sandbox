#pragma once

#include <vector>

#include "core/core.h"
#include "core/com_pointer.h"
#include "engine/renderer/command_list.h"

struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

namespace engine
{
  struct fdevice;

  enum ecommand_list_purpose : int
  {
    main = 0,
    ui,
    num
  };

  struct ENGINE_API fcommand_pair final
  {
    std::vector<fcom_ptr<ID3D12CommandAllocator>> command_allocator;  // index is back buffers id
    fgraphics_command_list command_list;   // TODO afaik I can use one command list for all back buffers, need to test it later
  };
  
  struct ENGINE_API fcommand_queue final
  {
    CTOR_DEFAULT(fcommand_queue)
    CTOR_MOVE_COPY_DELETE(fcommand_queue)
    fcommand_queue(fdevice* device, uint32_t in_back_buffer_count);
    ~fcommand_queue();
    
    
    void wait_for_fence_value(uint64_t value);
    bool is_fence_complete(uint64_t value) const;
    uint64_t signal();
    void flush();
    
    void reset_allocator(uint32_t back_buffer_id) const;

    fgraphics_command_list* get_command_list(ecommand_list_purpose type, uint32_t back_buffer_id);
    uint64_t execute_command_lists(uint32_t back_buffer_id);

    fcom_ptr<ID3D12CommandQueue> com;

  private: 
    std::vector<fcommand_pair> command_pairs; // index is ecommand_list_purpose
    
    uint32_t back_buffer_count = 0;
    HANDLE fence_event = nullptr;
    fcom_ptr<ID3D12Fence> fence;  
    std::vector<uint64_t> fence_value = {};
    uint64_t next_fence_value = 0;
  };
}