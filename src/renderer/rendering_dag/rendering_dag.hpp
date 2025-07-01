#pragma once

#include <misc/interfaces.hpp>
#include <misc/utils.hpp>

namespace TBD {

class RenderingDAG {
    TBD_NO_COPY_MOVE(RenderingDAG)
public:
    RenderingDAG() = default;

    template <RHI RHI>
    void render(RHI* rhi) const;

private:
};

}