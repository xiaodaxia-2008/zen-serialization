#pragma once
#include "Base.h"

#include <zenser_dummy_lib_derived_export.h>

#include <array>
#include <span>

class ZENSER_DUMMY_LIB_DERIVED_EXPORT DerivedNode : public BaseNode
{
public:
    DerivedNode(std::string name, std::array<float, 3> position)
        : BaseNode(std::move(name)), m_position(std::move(position))
    {
    }

    const std::array<float, 3> &GetPosition() const { return m_position; }

    void SetPosition(std::array<float, 3> position)
    {
        m_position = std::move(position);
    }

    ~DerivedNode();

    void serialize(OutArchive &ar) const;

    void serialize(InArchive &ar);

    void format(fmt::format_context &ctx) const override
    {
        auto parent = GetParent();
        BaseNode::format(ctx);
        fmt::format_to(ctx.out(), ", position: [{:.4g}, {:.4g}, {:.4g}]",
                       m_position[0], m_position[1], m_position[2]);
    }

protected:
    DerivedNode() = default;

    friend class zen::Access;

    std::array<float, 3> m_position;
};
