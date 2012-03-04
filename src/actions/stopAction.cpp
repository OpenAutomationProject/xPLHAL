#include "stopAction.h"

using std::string;
        
stopAction::stopAction(const pugi::xml_node& basenode)
: DeterminatorAction(basenode)
{
    parseFromXml(basenode);
}

void stopAction::execute() const
{
    throw AbortDeterminatorExecutionException();
}

