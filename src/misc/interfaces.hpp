#pragma once

#include <concepts>
#include <misc/types.hpp>

namespace TBD {

template <typename T>
concept IsValidTextureType = requires(T type) { { type.blit(T{}) } -> std::same_as<void>; }
    && requires(T type) {{type.clear(T{}, Color{})}-> std::same_as<void>; };

template <typename T>
concept IsValidBufferType = requires(T type) { { type.copy(T{}) } -> std::same_as<void>; };

template <typename T>
concept RHI = requires { typename T::TextureType; }
    && IsValidTextureType<typename T::TextureType>
    && requires { typename T::BufferType; }
// && IsValidBufferType<typename T::TextureType>
;

}