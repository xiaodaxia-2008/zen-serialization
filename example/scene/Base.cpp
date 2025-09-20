#include "Base.h"

BaseNode::~BaseNode() { delete m_number; }

REGISTER_CLASS(BaseNode)