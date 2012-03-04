#pragma once
#include "determinatoritems.h"

class executeAction: public DeterminatorAction<executeAction>
{
    public:
        executeAction() {}
        executeAction(const pugi::xml_node& basenode) {
            parseFromXml(basenode);
        }
        void execute() const;

        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string program;
        std::string parameters;
        std::string wait;
};
