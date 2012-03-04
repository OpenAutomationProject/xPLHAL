#pragma once
#include "determinatoritems.h"

class suspendAction: public DeterminatorAction<suspendAction>
{
    public:
        suspendAction() {}
        suspendAction(const pugi::xml_node& basenode) {
            parseFromXml(basenode);
        }
        void execute() const;

        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string suspendMinutes;
        std::string suspendTime;
        std::string suspendRandomise;
};
