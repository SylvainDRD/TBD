#pragma once

#include <misc/types.hpp>
#include <misc/utils.hpp>

namespace TBD {

// struct RenderingCommand {
//     virtual ~RenderingCommand() = default;
// };

// struct TextureBlit : RenderingCommand {
//     RID src;
//     RID dst;

//     template <RHI RHI>
//     void blit(RHI* rhi)
//     {
//         rhi->blit(src, dst);
//     }
// };

// struct TextureClear : RenderingCommand {
//     RID target;
//     Vec4 clearColor;

//     template <RHI RHI>
//     void clear(RHI* rhi)
//     {
//         typename RHI::TextureType texture = rhi->getTexture(target);

//         TBD_ASSERT(texture != nullptr, "Attempting to clear null texture");

//         texture->clear(clearColor);
//     }
// };

}