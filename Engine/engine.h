#pragma once

// Public header, Only one that should be included externally

#include "core/core.h"
#include "core/application.h"
#include "core/exceptions.h"

#include "engine/log.h"
#include "engine/io.h"
#include "engine/hash.h"
#include "engine/fpexcept.h"

#include "math/vec3.h"
#include "math/math.h"
#include "math/random.h"
#include "math/colors.h"
#include "math/ray.h"
#include "math/hit.h"
#include "math/random.h"
#include "math/aabb.h"
#include "math/colors.h"
#include "math/onb.h"
#include "math/pdf.h"
#include "math/vertex_data.h"
#include "math/camera.h"
#include "math/plane.h"
#include "object/object.h"
#include "object/object_registry.h"
#include "asset/soft_asset_ptr.h"
#include "assets/material.h"
#include "assets/mesh.h"
#include "assets/texture.h"
#include "assets/pixel_shader.h"
#include "assets/vertex_shader.h"
#include "resources/resources_io.h"
#include "persistence/persistence_helper.h"

#include "profile/stats.h"

#include "core/entry_point.h"
