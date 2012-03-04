#include "xplAction.h"

using std::string;
using std::shared_ptr;
using boost::regex;
        
xplAction::xplAction(const pugi::xml_node& basenode)
: DeterminatorAction(basenode)
{
    attributes["msg_type"]   = {shared_ptr<regex>(new regex(R"(^(stat|trig|cmnd)$)"))};
    attributes["msg_target"] = {shared_ptr<regex>(new regex(R"(^(\*|.*-.*\..*)$)"))};
    attributes["msg_schema"] = {shared_ptr<regex>(new regex(R"(^(.*\..*)$)"))};
    parseFromXml(basenode);
}

void xplAction::parseFromXml(const pugi::xml_node& basenode)
{
    DeterminatorAction::parseFromXml(basenode);
    for(const auto node : basenode) {
        if (node.name() == string("xplActionParam")) {
            actionParams.push_back(node.attribute("expression").value());
        }
    }
}

std::string xplAction::toString() const
{
    string ret = BaseDeterminatorItem::toString();
    for (auto ap : actionParams) {
        ret += "\nxplActionParam: " + ap;
    }
    return ret;
}

void xplAction::execute() const
{
}
