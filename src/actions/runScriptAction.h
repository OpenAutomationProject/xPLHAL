#pragma once
#include "determinatoritems.h"

class runScriptAction: public DeterminatorAction<runScriptAction>
{
    public:
        runScriptAction() {}
        runScriptAction(const pugi::xml_node& basenode) {
            parseFromXml(basenode);
        }
        void execute() const;

        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string scriptName;
        std::string parameter;
};

