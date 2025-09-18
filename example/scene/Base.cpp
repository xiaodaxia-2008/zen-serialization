#include "Base.h"

BaseNode::~BaseNode() { delete m_number; }

// template <class Archive>
// void BaseNode::serialize(Archive &ar)
// {
//     ar(m_name, m_parent, m_first_child_weak, m_first_child, m_children, m_id,
//     m_number, m_number1);
// }

// template ZPPBITS_DUMMY_LIB_BASE_EXPORT void BaseNode::serialize(OutArchive
// &ar); template ZPPBITS_DUMMY_LIB_BASE_EXPORT void
// BaseNode::serialize(InArchive &ar);

REGISTER_CLASS(BaseNode)