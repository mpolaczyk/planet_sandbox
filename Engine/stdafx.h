#pragma once

#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>
#include <iostream>
#include <string>
#include <format>

#include <DirectXMath.h>
#include <dxgi1_6.h>

#include "d3dx12/d3dx12.h"
#include "d3dx12/d3dx12_barriers.h"
#include "d3dx12/d3dx12_core.h"
#include "d3dx12/d3dx12_root_signature.h"
#include "d3dx12/d3dx12_resource_helpers.h"

#include "core/core.h"
#include "core/windows_minimal.h"
#include "core/exceptions/windows_error.h"
#include "core/application.h"
#include "core/rtti/object_registry.h"
#include "core/rtti/object_visitor.h"
#include "engine/log.h"

#include "nlohmann/json.hpp"