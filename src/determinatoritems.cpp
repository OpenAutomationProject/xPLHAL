#include "determinatoritems.h"
#include <cxxabi.h>
#include <iostream>
#include <typeinfo>
#include <iostream>
#include "log.h"

using std::string;
using std::vector;
using std::map;

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
        ScopedXmlAttributeGetter(const pugi::xml_node& basenode, map<string, string> &attributemap) 
        :m_basenode(basenode),m_attributemap(attributemap) {}
        string get(const string& attribute_name) const {
            pugi::xml_attribute xml_attribute = m_basenode.attribute(attribute_name.c_str());
            if (!xml_attribute) {
                string error_text = string("In node '") + m_basenode.name() + "'";
                error_text += " attribute '" + attribute_name + "' was not found";
                throw DeterminatorParseException(error_text);
            }
            m_attributemap.insert({attribute_name, xml_attribute.value()});
            return xml_attribute.value();
        }
    private:
        const pugi::xml_node& m_basenode;
        map<string, string> &m_attributemap;
};

BaseDeterminatorItem::BaseDeterminatorItem(const string& name)
:item_name(name) 
{ 
}

BaseDeterminatorItem::BaseDeterminatorItem(const pugi::xml_node& basenode, const string& name)
:item_name(name)
{
    ScopedXmlAttributeGetter a(basenode, attributes);
    display_name = a.get("display_name");
}
        
bool BaseDeterminatorItem::match() const
{
    return true;
}

void BaseDeterminatorItem::execute() const
{
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
    ScopedXmlAttributeGetter helper(basenode, attributes);
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
            ScopedXmlAttributeGetter pa(node, attributes);
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
    ScopedXmlAttributeGetter helper(basenode, attributes);
    name  = helper.get("name");
    op    = helper.get("operator");
    value = helper.get("value");
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
    ScopedXmlAttributeGetter helper(basenode, attributes);
    name  = helper.get("name");
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
    ScopedXmlAttributeGetter helper(basenode, attributes);
    dow  = helper.get("dow");
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
    ScopedXmlAttributeGetter helper(basenode, attributes);
    op    = helper.get("operator");
    value = helper.get("value");
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

void logAction::parseFromXml(const pugi::xml_node& basenode)
{
    ScopedXmlAttributeGetter helper(basenode, attributes);
    logText      = helper.get("logText");
    executeOrder = helper.get("executeOrder");
}

std::string logAction::toString() const
{
    string ret = item_name + ":";
    ret += "\nlogText.....: " + logText;
    ret += "\nexecuteOrder: " + executeOrder;
    return ret;
}

void logAction::execute() const
{
    writeLog(logText, logLevel::debug);
}

//-----

void xplAction::parseFromXml(const pugi::xml_node& basenode)
{
    ScopedXmlAttributeGetter helper(basenode, attributes);
    executeOrder = helper.get("executeOrder");
    msgType = helper.get("msgType");
    msgTarget = helper.get("msgTarget");
    msgSchema = helper.get("msgSchema");
    
    for(const auto node : basenode) {
        if (node.name() == string("xplActionParam")) {
            string expression = node.attribute("expression").value();
        }
    }
}

std::string xplAction::toString() const
{
    string ret = item_name + ":";
    ret += "\nexecuteOrder: " + executeOrder;
    ret += "\nmsgType.....: " + msgType;
    ret += "\nmsgTarget...: " + msgTarget;
    ret += "\nmsgSchema...: " + msgSchema;
    return ret;
}

void xplAction::execute() const
{
}

