// Node1.cpp: implementation of the Node class.
//
//////////////////////////////////////////////////////////////////////


#include "Node.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Node::Node() {

}

Node::~Node() {

}

string Node::getName() const {
    return m_name;
}


void Node::setName(const string name) {
    m_name = name;
}
