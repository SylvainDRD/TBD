#pragma once

#include <concepts>
#include <functional>
#include <misc/utils.hpp>
#include <renderer/core/rhi_interface.hpp>

namespace TBD {

class RenderingDAG;

class IRHI {
    TBD_NO_COPY_MOVE(IRHI)
public:
    IRHI() = default;

    virtual ~IRHI() = default;

    virtual void render(const RenderingDAG& dag) const = 0;
};

template <typename T>
concept Releasable = requires(T resource, const IRHI& rhi) { { resource.release(rhi) } -> std::same_as<void>; };

template <typename T>
concept UnmanagedResource = std::movable<T> && Releasable<T>;

// TODO: figure out what to do with the command buffers
template <typename RHI>
concept HasTexture = UnmanagedResource<typename RHI::TextureType>;
// && requires(RHI::TextureType texture, RHI::TextureType& other, std::any cmd) { { texture.blit(cmd, other) } -> std::same_as<void>; }
// && requires(RHI::TextureType texture, std::any cmd) { { texture.clear(cmd, Color{}) } -> std::same_as<void>; };

// TODO
template <typename RHI>
concept HasBuffer = requires { typename RHI::BufferType; }; // requires UnmanagedResource && requires(T buffer) { { buffer.copy(T{}) } -> std::same_as<void>; };

template <typename T>
concept RHI = std::derived_from<T, IRHI> && HasTexture<T> && HasBuffer<T>;

}