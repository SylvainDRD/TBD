#pragma once

#include <misc/utils.hpp>
#include <renderer/rendering_dag/rendering_commands/rendering_commands.hpp>
#include <renderer/core/rhi_interface.hpp>

namespace TBD {

class RenderingDAG {
    TBD_NO_COPY_MOVE(RenderingDAG)
public:
    RenderingDAG() = default;

    template <RHI RHI>
    void render(RHI* rhi) const;

    void clear();

private:
    // std::vector<RenderingCommand*> _commands;
};

}