#pragma once
#include "determinatoritems.h"

class globalCondition: public DeterminatorCondition<globalCondition>
{
    public:
        globalCondition() {}
        globalCondition(const pugi::xml_node& basenode);
};

