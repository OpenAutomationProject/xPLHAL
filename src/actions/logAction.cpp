#include "logAction.h"
#include "log.h"

using std::string;

logAction::logAction(const pugi::xml_node& basenode)
: DeterminatorAction(basenode)
{
    attributes["logText"];
    parseFromXml(basenode);
}

void logAction::execute() const
{
    writeLog(getAttribute("logText"), logLevel::debug);
}

