#include "globalAction.h"
#include <iostream>

using std::string;
using std::cerr;
            
globalAction::globalAction(const pugi::xml_node& basenode)
{
    attributes["name"];
    attributes["value"];
    parseFromXml(basenode);
}

void globalAction::execute() const
{
    std::cerr << "globalAction set " << getAttribute("name") << " to " << getAttribute("value") << "\n";
}

