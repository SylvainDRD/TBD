#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace TBD {

using int64 = int64_t;
using uint64 = uint64_t;
using int32 = int32_t;
using uint32 = uint32_t;
using byte = unsigned char;

template <class T>
using Uptr = std::unique_ptr<T>;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;

} // namespace TBD