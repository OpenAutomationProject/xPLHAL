#pragma once
#include "determinatoritems.h"

class globalAction: public DeterminatorAction<globalAction>
{
    public:
        globalAction() {}
        globalAction(const pugi::xml_node& basenode);

        void execute() const;
};

