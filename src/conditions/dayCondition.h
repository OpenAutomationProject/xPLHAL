#pragma once
#include "determinatoritems.h"

class dayCondition: public DeterminatorCondition<dayCondition>
{
    public:
        dayCondition() {}
        dayCondition(const pugi::xml_node& basenode);
};

