#include "dayCondition.h"

using std::string;
        
dayCondition::dayCondition(const pugi::xml_node& basenode)
{
    attributes["dow"];
    parseFromXml(basenode);
}

