#pragma once
#include "determinatoritems.h"

class xplCondition: public DeterminatorCondition<xplCondition>
{
    public:
        xplCondition() {}
        xplCondition(const pugi::xml_node& basenode);
        
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;
        /* connect to signal of new xpl-message */

        struct parameter {
            std::string name;
            std::string op;
            std::string value;
        };

        std::vector<struct parameter> parameter;
};

