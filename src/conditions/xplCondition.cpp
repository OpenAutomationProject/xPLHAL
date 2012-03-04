#include "xplCondition.h"
#include <iostream>

using std::string;
using std::cerr;

xplCondition::xplCondition(const pugi::xml_node& basenode)
{
    attributes["msg_type"];
    attributes["source_vendor"];
    attributes["source_device"];
    attributes["source_instance"];
    attributes["target_vendor"];
    attributes["target_device"];
    attributes["target_instance"];
    attributes["schema_class"];
    attributes["schema_type"];
    parseFromXml(basenode);
}

void xplCondition::parseFromXml(const pugi::xml_node& basenode) 
{
    BaseDeterminatorItem::parseFromXml(basenode);
    for(const auto node : basenode) {
        if (node.name() == string("param")) {
            struct parameter p;
            p.name  = node.attribute("name").value();
            p.op    = node.attribute("operator").value();
            p.value = node.attribute("value").value();
            parameter.push_back(p);
        }
    }
}
        
string xplCondition::toString() const 
{
    string ret = BaseDeterminatorItem::toString();
    for (auto p : parameter) {
        ret +="\nparameter: " + p.name + p.op + p.value;
    }
    return ret;
}

