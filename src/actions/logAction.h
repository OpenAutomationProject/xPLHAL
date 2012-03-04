#pragma once
#include "determinatoritems.h"

class logAction: public DeterminatorAction<logAction>
{
    public:
        logAction() {}
        logAction(const pugi::xml_node& basenode);

        void execute() const;
};

