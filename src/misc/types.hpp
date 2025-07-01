#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <memory>
#include <misc/utils.hpp>

namespace TBD {

using RIDType = uint32_t;
using RID = RIDType;
static constexpr RID InvalidRID = std::numeric_limits<RID>::max();

template <class T>
using Uptr = std::unique_ptr<T>;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Color = Vec4;
using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;

} // namespace TBD