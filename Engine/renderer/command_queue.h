#pragma once

#include <vector>
#include <wrl/client.h>

#include "core/core.h"

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

using Microsoft::WRL::ComPtr;

namespace engine
{
  enum ecommand_list_type : int
  {
    main = 0,
    ui,
    num
  };

  struct ENGINE_API fcommand_pair
  {
    std::vector<ComPtr<ID3D12CommandAllocator>> command_allocator;  // For backbuffers
    ComPtr<ID3D12GraphicsCommandList> command_list;
  };
  
  struct ENGINE_API fcommand_queue
  {
  public:
    void init(ComPtr<ID3D12Device> device, uint32_t in_back_buffer_count);
    void cleanup();
    
    void wait_for_fence_value(uint64_t value) const;
    bool is_fence_complete(uint64_t value) const;
    uint64_t signal();
    void flush();

    ComPtr<ID3D12CommandQueue> get_command_queue() const;
    void close_command_lists(uint32_t back_buffer_id);
    void reset_command_lists(uint32_t back_buffer_id);
    ComPtr<ID3D12GraphicsCommandList> get_command_list(ecommand_list_type type, uint32_t back_buffer_id) const;
    uint64_t execute_command_lists(uint32_t back_buffer_id);
    
  private: 
    ComPtr<ID3D12CommandQueue> command_queue;
    std::vector<fcommand_pair> command_pair; // index is ecommand_list_type
    
    uint32_t back_buffer_count = 0;
    HANDLE fence_event = nullptr;
    ComPtr<ID3D12Fence> fence;  
    std::vector<uint64_t> fence_value = {};
    uint64_t last_fence_value = 0;
  };
}