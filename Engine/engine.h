#pragma once

// Public header, Only one that should be included externally

#include "core/core.h"
#include "core/application.h"
#include "core/exceptions/windows_error.h"

#include "engine/log.h"
#include "engine/io.h"
#include "engine/math/hash.h"
#include "core/exceptions/floating_point.h"

#include "engine/math/vec3.h"
#include "engine/math/math.h"
#include "engine/math/random.h"
#include "engine/math/tone_mapping.h"
#include "engine/math/ray.h"
#include "engine/math/hit.h"
#include "engine/math/random.h"
#include "engine/math/aabb.h"
#include "engine/math/tone_mapping.h"
#include "engine/math/vertex_data.h"
#include "engine/math/camera.h"
#include "core/rtti/object.h"
#include "core/rtti/object_registry.h"
#include "engine/asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "engine/resources/resources_io.h"
#include "engine/persistence/persistence_helper.h"


#include "core/entry_point.h"
