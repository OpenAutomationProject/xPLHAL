#include "determinatoritems.h"
#include <cxxabi.h>
#include <iostream>
#include <typeinfo>
#include <iostream>

using std::string;
using std::vector;

ConditionParseException::ConditionParseException(const string& text)
:m_text(text) 
{
}
        
const char* ConditionParseException::what() const throw() 
{
    return m_text.c_str();

}

DeterminatorParseException::DeterminatorParseException(const string& text)
:m_text(text)
{
}

const char* DeterminatorParseException::what() const throw() 
{
    return m_text.c_str();
}


class ScopedXmlAttributeGetter 
{
    public:
        ScopedXmlAttributeGetter(const pugi::xml_node& basenode) :m_basenode(basenode) {}
        string get(const string& attribute_name) const {
            pugi::xml_attribute xml_attribute = m_basenode.attribute(attribute_name.c_str());
            if (!xml_attribute) {
                string error_text = string("In node '") + m_basenode.name() + "'";
                error_text += " attribute '" + attribute_name + "' was not found";
                throw ConditionParseException(error_text);
            }
            return xml_attribute.value();
        }
    private:
        const pugi::xml_node& m_basenode;
};

BaseDeterminatorItem::BaseDeterminatorItem(const string& name)
:item_name(name) 
{ 
}

BaseDeterminatorItem::BaseDeterminatorItem(const pugi::xml_node& basenode, const string& name)
:item_name(name)
{
    ScopedXmlAttributeGetter a(basenode);
    display_name = a.get("display_name");
}

/*
 * Determinator Conditions
 */

XplCondition::XplCondition()
:BaseDeterminatorItem("xplCondition")
{
}

XplCondition::XplCondition(const pugi::xml_node& basenode)
:BaseDeterminatorItem(basenode, "xplCondition")
{
    parseFromXml(basenode);
}
        
BaseDeterminatorItemPtr XplCondition::createNew(const pugi::xml_node& basenode) const 
{
    return BaseDeterminatorItemPtr(new XplCondition(basenode));
}

void XplCondition::parseFromXml(const pugi::xml_node& basenode) 
{
    ScopedXmlAttributeGetter helper(basenode);
    msg_type        = helper.get("msg_type");
    source_vendor   = helper.get("source_vendor");
    source_device   = helper.get("source_device");
    source_instance = helper.get("source_instance");
    target_vendor   = helper.get("target_vendor");
    target_device   = helper.get("target_device");
    target_instance = helper.get("target_instance");
    schema_class    = helper.get("schema_class");
    schema_type     = helper.get("schema_type");

    for(const auto node : basenode) {
        if (node.name() == string("param")) {
            struct parameter p;
            ScopedXmlAttributeGetter pa(node);
            p.name  = pa.get("name");
            p.op    = pa.get("operator");
            p.value = pa.get("value");
            parameter.push_back(p);
        }
    }
}
        
string XplCondition::toString() const 
{
    string ret = "xplCondition:";
    ret += "\nmsg_type: " + msg_type;
    ret += "\nsource_vendor: " + source_vendor;
    ret += "\nsource_device: " + source_device;
    ret += "\nsource_instance: " + source_instance;
    ret += "\ntarget_vendor: " + target_vendor;
    ret += "\ntarget_device: " + target_device;
    ret += "\ntarget_instance: " + target_instance;
    ret += "\nschema_class: " + schema_class;
    ret += "\nschema_type: " + schema_type;
    for (auto p : parameter) {
        ret +="\nparameter: " + p.name + p.op + p.value;
    }
    return ret;
}


GlobalCondition::GlobalCondition() 
:BaseDeterminatorItem("globalCondition")
{
}

GlobalCondition::GlobalCondition(const pugi::xml_node& basenode)
:BaseDeterminatorItem(basenode, "globalCondition")
{
    parseFromXml(basenode);
}

BaseDeterminatorItemPtr GlobalCondition::createNew(const pugi::xml_node& basenode) const 
{
    return BaseDeterminatorItemPtr(new GlobalCondition(basenode));
}

void GlobalCondition::parseFromXml(const pugi::xml_node& basenode) 
{
    name  = basenode.attribute("name").value();
    op    = basenode.attribute("operator").value();
    value = basenode.attribute("value").value();
}
        
string GlobalCondition::toString() const 
{
    string ret = item_name + ":";
    ret += "\nname: " + name;
    ret += "\noperator: " + op;
    ret += "\nvalue: " + value;
    return ret;
}

GlobalChanged::GlobalChanged() 
:BaseDeterminatorItem("globalChanged")
{
}

GlobalChanged::GlobalChanged(const pugi::xml_node& basenode)
:BaseDeterminatorItem(basenode, "globalChanged")
{
    parseFromXml(basenode);
}

BaseDeterminatorItemPtr GlobalChanged::createNew(const pugi::xml_node& basenode) const 
{
    return BaseDeterminatorItemPtr(new GlobalChanged(basenode));
}

void GlobalChanged::parseFromXml(const pugi::xml_node& basenode) 
{
    name  = basenode.attribute("name").value();
}

string GlobalChanged::toString() const 
{
    string ret = item_name + ":";
    ret += "\nname: " + name;
    return ret;
}


DayCondition::DayCondition() 
:BaseDeterminatorItem("dayCondition")
{
}

DayCondition::DayCondition(const pugi::xml_node& basenode)
:BaseDeterminatorItem(basenode, "dayCondition")
{
    parseFromXml(basenode);
}

BaseDeterminatorItemPtr DayCondition::createNew(const pugi::xml_node& basenode) const 
{
    return BaseDeterminatorItemPtr(new DayCondition(basenode));
}

void DayCondition::parseFromXml(const pugi::xml_node& basenode) 
{
    dow  = basenode.attribute("dow").value();
}

string DayCondition::toString() const 
{
    string ret = item_name + ":";
    ret += "\ndow: " + dow;
    return ret;
}

TimeCondition::TimeCondition() 
:BaseDeterminatorItem("timeCondition")
{
}

TimeCondition::TimeCondition(const pugi::xml_node& basenode)
    :BaseDeterminatorItem(basenode, "timeCondition")
{
    parseFromXml(basenode);
}

BaseDeterminatorItemPtr TimeCondition::createNew(const pugi::xml_node& basenode) const 
{
    return BaseDeterminatorItemPtr(new TimeCondition(basenode));
}

void TimeCondition::parseFromXml(const pugi::xml_node& basenode) 
{
    op  = basenode.attribute("operator").value();
    value = basenode.attribute("value").value();
}

string TimeCondition::toString() const 
{
    string ret = item_name + ":";
    ret += "\noperator: " + op;
    ret += "\nvalue...: " + value;
    return ret;
}

/*
 * Determinator Actions
 */

LogAction::LogAction()
:BaseDeterminatorItem("logAction")
{
}

LogAction::LogAction(const pugi::xml_node& basenode)
:BaseDeterminatorItem(basenode, "logAction")
{
    parseFromXml(basenode);
}

BaseDeterminatorItemPtr LogAction::createNew(const pugi::xml_node& basenode) const
{
    return BaseDeterminatorItemPtr(new LogAction(basenode));
}

void LogAction::parseFromXml(const pugi::xml_node& basenode)
{
    logText = basenode.attribute("logText").value();
    executeOrder = basenode.attribute("executeOrder").value();
}

std::string LogAction::toString() const
{
    string ret = item_name + ":";
    ret += "\nlogText.....: " + logText;
    ret += "\nexecuteOrder: " + executeOrder;
    return ret;
}

