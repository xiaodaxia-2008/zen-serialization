#include "derived_node.h"

DerivedNode::~DerivedNode() {}

void DerivedNode::serialize(OutArchive &ar) const
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_position));
}

void DerivedNode::serialize(InArchive &ar)
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_position));
}

REGISTER_CLASS(DerivedNode)
