#include "globalChanged.h"

using std::string;
        
globalChanged::globalChanged(const pugi::xml_node& basenode)
{
    attributes["name"];
    parseFromXml(basenode);
}

