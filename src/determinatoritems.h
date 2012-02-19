#pragma once
#include "pugixml.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>

class DeterminatorParseException: public std::exception
{
    public:
        DeterminatorParseException(const std::string& text);
        virtual ~DeterminatorParseException() throw() {}
        const char* what() const throw();
    private:
        std::string m_text;
};

class BaseDeterminatorItem;
typedef std::shared_ptr<BaseDeterminatorItem> BaseDeterminatorItemPtr;
typedef std::shared_ptr<const BaseDeterminatorItem> BaseDeterminatorItemConstPtr;

class BaseDeterminatorItem
{
    public:
        BaseDeterminatorItem(const std::string& name);
        BaseDeterminatorItem(const pugi::xml_node& basenode, const std::string& name);
        
        virtual void parseFromXml(const pugi::xml_node& basenode) = 0;
        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const = 0;
        virtual std::string toString() const = 0;
        virtual bool match() const;
        virtual void execute() const;

        std::string item_name;
        std::string display_name;

        std::map<std::string, std::string> attributes;

        //boost::signal2::signal<void ()> sigChanged;
};

template<typename T>
class DeterminatorAction: public BaseDeterminatorItem
{
    public:
        DeterminatorAction() :BaseDeterminatorItem(typeid(T).name()) { }
        DeterminatorAction(const pugi::xml_node& basenode) 
        :BaseDeterminatorItem(basenode, std::string(typeid(T).name())) { 
            executeOrder = basenode.attribute("executeOrder").value();
            attributes["executeOrder"] = executeOrder;
        }
        
        BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const {
            return BaseDeterminatorItemPtr(new T(basenode));
        }
        
        std::string executeOrder;
};


/*
 * Conditions
 */

class XplCondition: public BaseDeterminatorItem
{
    public:
        XplCondition();
        XplCondition(const pugi::xml_node& basenode);
        
        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;

        void parseFromXml(const pugi::xml_node& basenode);
        
        std::string toString() const;

        /* connect to signal of new xpl-message */
        std::string msg_type;
        std::string source_vendor;
        std::string source_device;
        std::string source_instance;
        std::string target_vendor;
        std::string target_device;
        std::string target_instance;
        std::string schema_class;
        std::string schema_type;

        struct parameter {
            std::string name;
            std::string op;
            std::string value;
        };

        std::vector<struct parameter> parameter;
};

class GlobalCondition: public BaseDeterminatorItem
{
    public:
        GlobalCondition();
        GlobalCondition(const pugi::xml_node& basenode);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;

        void parseFromXml(const pugi::xml_node& basenode);

        std::string toString() const;

        std::string name;
        std::string op;
        std::string value;
};

class GlobalChanged: public BaseDeterminatorItem
{
    public:
        /* connect to signal of changed global variable */
        GlobalChanged();
        GlobalChanged(const pugi::xml_node& basenode);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string name;
};

class DayCondition: public BaseDeterminatorItem
{
    public:
        DayCondition();
        DayCondition(const pugi::xml_node& basenode);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string dow;
};

class TimeCondition: public BaseDeterminatorItem
{
    public:
        TimeCondition();
        TimeCondition(const pugi::xml_node& basenode);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string op;
        std::string value;
};

/*
 * Actions
 */

class logAction: public DeterminatorAction<logAction>
{
    public:
        logAction() {}
        logAction(const pugi::xml_node& basenode) : DeterminatorAction(basenode) {
            parseFromXml(basenode);
        }

        void execute() const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string logText;
        std::string executeOrder;
};

class xplAction: public DeterminatorAction<xplAction>
{
    public:
        xplAction() {}
        xplAction(const pugi::xml_node& basenode) : DeterminatorAction(basenode) {
            parseFromXml(basenode);
        }

        void execute() const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string msgType;
        std::string msgTarget;
        std::string msgSchema;
        std::multimap<std::string, std::string> actionParams;
};

class globalAction: public DeterminatorAction<globalAction>
{
    public:
        globalAction();
        globalAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string name;
        std::string value;
};

class delayAction: public DeterminatorAction<delayAction>
{
    public:
        delayAction();
        delayAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string delaySeconds;
};

class stopAction: public DeterminatorAction<stopAction>
{
    public:
        stopAction();
        stopAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

};

class suspendAction: public DeterminatorAction<suspendAction>
{
    public:
        suspendAction();
        suspendAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string suspendMinutes;
        std::string suspendTime;
        std::string suspendRandomise;
};

class execRuleAction: public DeterminatorAction<execRuleAction>
{
    public:
        execRuleAction();
        execRuleAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string ruleName;
};

class runScriptAction: public DeterminatorAction<runScriptAction>
{
    public:
        runScriptAction();
        runScriptAction(const pugi::xml_node& basenode);
        void execute() const;

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string scriptName;
        std::string parameter;
};

