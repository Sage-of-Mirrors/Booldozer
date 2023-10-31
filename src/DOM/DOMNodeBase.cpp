#include "DOM/DOMNodeBase.hpp"

static int32_t NextNodeID = 0;

LDOMNodeBase::LDOMNodeBase(std::string name) { 
    mName = name; 
    SetIsSelected(false); 
    SetIsRendered(true); 
    SetIsInitialized(false); 
    mNodeID = NextNodeID++;
}