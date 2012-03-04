#pragma once
#include "determinatoritems.h"

class dayCondition: public DeterminatorCondition<dayCondition>
{
    public:
        dayCondition() {}
        dayCondition(const BaseDeterminatorItem::attribute_t& attrlist);
        dayCondition(const pugi::xml_node& basenode);

        bool match() const;
};

