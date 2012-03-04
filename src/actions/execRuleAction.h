#pragma once
#include "determinatoritems.h"

class execRuleAction: public DeterminatorAction<execRuleAction>
{
    public:
        execRuleAction() {}
        execRuleAction(const pugi::xml_node& basenode) {
            parseFromXml(basenode);
        }
        void execute() const;

        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string ruleName;
};
