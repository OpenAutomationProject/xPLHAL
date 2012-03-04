#pragma once
#include "determinatoritems.h"

class globalChanged: public DeterminatorCondition<globalChanged>
{
    public:
        /* connect to signal of changed global variable */
        globalChanged() {}
        globalChanged(const pugi::xml_node& basenode);
};

