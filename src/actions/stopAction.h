#pragma once
#include "determinatoritems.h"

class stopAction: public DeterminatorAction<stopAction>
{
    public:
        stopAction() {}
        stopAction(const pugi::xml_node& basenode);
        void execute() const;
};

