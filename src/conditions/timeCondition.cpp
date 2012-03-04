#include "timeCondition.h"
#include <boost/regex.hpp>
#include <memory>
#include <map>

using std::string;
using std::shared_ptr;
using std::map;
using boost::regex;
using boost::regex_match;
        
timeCondition::timeCondition(const pugi::xml_node& basenode)
{
    attributes["operator"] = {shared_ptr<regex>(new regex(R"(^(=|!=|<|>|<=|>=)$)"))};
    attributes["value"]    = {shared_ptr<regex>(new regex(R"(^\d{1,2}:\d{1,2}$)"))};

    parseFromXml(basenode);
}

bool timeCondition::match() const
{
}
