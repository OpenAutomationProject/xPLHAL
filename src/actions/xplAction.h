#pragma once
#include "determinatoritems.h"

class xplAction: public DeterminatorAction<xplAction>
{
    public:
        xplAction() {}
        xplAction(const pugi::xml_node& basenode);

        void execute() const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::list<std::string> actionParams;
};

