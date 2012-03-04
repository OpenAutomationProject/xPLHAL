#include "delayAction.h"

using std::string;
using std::vector;
using std::map;
        
delayAction::delayAction(const pugi::xml_node& basenode)
{
    attributes["delay_seconds"];
    parseFromXml(basenode);
}

void delayAction::execute() const
{
    sleep(atoi(getAttribute("delaySeconds").c_str()));
}

