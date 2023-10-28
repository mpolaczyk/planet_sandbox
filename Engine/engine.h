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
#include "math/triangle_face.h"
#include "math/camera.h"
#include "math/plane.h"
#include "asset/asset.h"
#include "asset/soft_asset_ptr.h"
#include "asset/asset_registry.h"
#include "asset/materials.h"
#include "asset/mesh.h"
#include "asset/textures.h"
#include "third_party/asset_tools.h"

#include "profile/stats.h"

#include "core/entry_point.h"