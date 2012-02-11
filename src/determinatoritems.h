#pragma once
#include "pugixml.hpp"
#include <string>
#include <vector>
#include <memory>

class ConditionParseException: public std::exception
{
    public:
        ConditionParseException(const std::string& text);
        virtual ~ConditionParseException() throw() {}
        const char* what() const throw();
    private:
        std::string m_text;
};

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

        std::string item_name;
        std::string display_name;

        //boost::signal2::signal<void ()> sigChanged;
};

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

class LogAction: public BaseDeterminatorItem
{
    public:
        LogAction();
        LogAction(const pugi::xml_node& basenode);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const;
        void parseFromXml(const pugi::xml_node& basenode);
        std::string toString() const;

        std::string logText;
        std::string executeOrder;
};
