#include "globalCondition.h"

using std::string;
using std::shared_ptr;
using boost::regex;
        
globalCondition::globalCondition(const pugi::xml_node& basenode)
{
    attributes["name"];
    attributes["operator"] = {shared_ptr<regex>(new regex(R"(^(=|!=|<|>|<=|>=)$)"))};
    attributes["value"];
    parseFromXml(basenode);
}
