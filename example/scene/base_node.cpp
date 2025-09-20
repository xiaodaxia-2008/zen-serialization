#include "base_node.h"

BaseNode::~BaseNode() { delete m_number; }

REGISTER_CLASS(BaseNode)