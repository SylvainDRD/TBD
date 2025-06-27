#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace TBD {

template <class T>
using Uptr = std::unique_ptr<T>;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;

} // namespace TBD