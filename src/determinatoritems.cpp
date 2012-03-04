#include "determinatoritems.h"
#include <cxxabi.h>
#include <iostream>
#include <typeinfo>
#include <iostream>
#include "log.h"

using std::string;
using std::vector;
using std::map;
using std::cerr;

DeterminatorParseException::DeterminatorParseException(const string& text)
:m_text(text)
{
}

const char* DeterminatorParseException::what() const throw() 
{
    return m_text.c_str();
}

BaseDeterminatorItem::BaseDeterminatorItem(const string& name)
:item_name(name) 
{ 
    attributes["display_name"] = {};
}

BaseDeterminatorItem::BaseDeterminatorItem(const pugi::xml_node& basenode, const string& name)
:item_name(name)
{
    attributes["display_name"] = {};
}
        
void BaseDeterminatorItem::parseFromXml(const pugi::xml_node& basenode)
{
    for (auto &ai: attributes) {
        const string attribute_name = ai.first;
        pugi::xml_attribute xml_attribute = basenode.attribute(attribute_name.c_str());

        if (!xml_attribute) {
            string error_text = string("In node '") + basenode.name() + "'";
            error_text += " attribute '" + attribute_name + "' was not found";
            throw DeterminatorParseException(error_text);
        }

        ai.second.value = xml_attribute.value();
        
        if (ai.second.re.get() != nullptr) {
            if (! regex_match(ai.second.value, *ai.second.re)) {
                string error_text = string("In node '") + basenode.name() + "'";
                error_text += " invalid value for attribute '" + ai.first + "' value: '" + ai.second.value + "'.";
                throw DeterminatorParseException(error_text);
            }
        }
    }
}
        
bool BaseDeterminatorItem::match() const
{
    return true;
}

void BaseDeterminatorItem::execute() const
{
}

std::string BaseDeterminatorItem::getAttribute(const std::string& attrname) const
{
    auto iter = attributes.find(attrname);
    if (iter == attributes.end()) {
        return "";
    }
    return iter->second.value;
}

string BaseDeterminatorItem::toString() const
{
    string ret = item_name + ":";
    for (auto iter : attributes) {
        ret += "\n" + iter.first + ": " + iter.second.value;
    }
    return ret;
}

