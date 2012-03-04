#pragma once
#include "determinatoritems.h"

class timeCondition: public DeterminatorCondition<timeCondition>
{
    public:
        timeCondition() {}
        timeCondition(const pugi::xml_node& basenode);

        virtual bool match() const;
};

