#include "Derived.h"

DerivedNode::~DerivedNode() {}

void DerivedNode::save(OutArchive &ar) const
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_position));
}

void DerivedNode::load(InArchive &ar)
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_position));
}

REGISTER_CLASS(DerivedNode)
