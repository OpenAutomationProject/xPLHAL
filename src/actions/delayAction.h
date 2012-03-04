#pragma once
#include "determinatoritems.h"

class delayAction: public DeterminatorAction<delayAction>
{
    public:
        delayAction() {}
        delayAction(const pugi::xml_node& basenode);

        void execute() const;
};
