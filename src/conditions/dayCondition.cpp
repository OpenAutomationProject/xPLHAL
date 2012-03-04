#include "dayCondition.h"
#include <ctime>
#include <chrono>
#include <iostream>

using std::string;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::shared_ptr;
using boost::regex;
using std::cout;
        
dayCondition::dayCondition(const BaseDeterminatorItem::attribute_t& attrlist)
{
    attributes = attrlist;
}

dayCondition::dayCondition(const pugi::xml_node& basenode)
{
    attributes["dow"] = {shared_ptr<regex>(new regex(R"(^[01]{7}$)"))};
    parseFromXml(basenode);
}

bool dayCondition::match() const 
{
    std::time_t currentTime = std::time(0);
    auto tm = std::localtime(&currentTime);
    string dow_config = getAttribute("dow");

    if (dow_config.length() == 7) {
        return dow_config[tm->tm_wday] == '1';
    }
    return false;
}
