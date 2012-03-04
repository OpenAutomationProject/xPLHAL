#include "timeCondition.h"
#include <chrono>

using std::string;
using std::shared_ptr;
using std::chrono::time_point;
using std::chrono::duration;
using std::chrono::system_clock;
using boost::regex;
using boost::regex_match;
using boost::smatch;

        
timeCondition::timeCondition(const BaseDeterminatorItem::attribute_t& attrlist, std::time_t fixTime)
:mTestTime(fixTime)
{
    attributes["operator"] = {shared_ptr<regex>(new regex(R"(^(=|!=|<|>|<=|>=)$)"))};
    attributes["value"]    = {shared_ptr<regex>(new regex(R"(^(\d{1,2}):(\d{1,2})$)"))};

    for (auto a : attrlist) {
        attributes[a.first].value = a.second.value;
    }

    setCompareTime();
}
        
timeCondition::timeCondition(const pugi::xml_node& basenode)
:mTestTime(0)
{
    attributes["operator"] = {shared_ptr<regex>(new regex(R"(^(=|!=|<|>|<=|>=)$)"))};
    attributes["value"]    = {shared_ptr<regex>(new regex(R"(^(\d{1,2}):(\d{1,2})$)"))};

    parseFromXml(basenode);
    setCompareTime();
}

void timeCondition::setCompareTime()
{
    smatch res;

    if (! regex_match(attributes["value"].value, res, *attributes["value"].re)) {
        string error_text = "While creating timeCondition,";
        error_text += " error getting value for attribute 'value': regex_match failed";
        throw DeterminatorParseException(error_text);
    }

    mCompareHour = atoi(res.str(1).c_str());
    mCompareMinute = atoi(res.str(2).c_str());
}

bool timeCondition::match() const
{
    /* for testing use mTestTime if != 0 */
    std::time_t now = mTestTime ? mTestTime : std::time(0);
    struct tm tm;
    localtime_r(&now, &tm);

    tm.tm_hour = mCompareHour;
    tm.tm_min  = mCompareMinute;

    auto tp_now = system_clock::from_time_t(now);
    auto tp_cmp = system_clock::from_time_t(std::mktime(&tm));

    string op = getAttribute("operator");

    if (op == "=")       return tp_now == tp_cmp;
    else if (op == "!=") return tp_now != tp_cmp;
    else if (op == "<")  return tp_now <  tp_cmp;
    else if (op == ">")  return tp_now >  tp_cmp;
    else if (op == "<=") return tp_now <= tp_cmp;
    else if (op == ">=") return tp_now >= tp_cmp;
    return false;
}
