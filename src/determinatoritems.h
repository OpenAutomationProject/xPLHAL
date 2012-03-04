#pragma once
#include "pugixml.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cxxabi.h>
#include <boost/regex.hpp>

class DeterminatorParseException: public std::exception
{
    public:
        DeterminatorParseException(const std::string& text);
        virtual ~DeterminatorParseException() throw() {}
        const char* what() const throw();
    private:
        std::string m_text;
};
    
class AbortDeterminatorExecutionException: public std::exception {};

class BaseDeterminatorItem;
typedef std::shared_ptr<BaseDeterminatorItem> BaseDeterminatorItemPtr;
typedef std::shared_ptr<const BaseDeterminatorItem> BaseDeterminatorItemConstPtr;

class BaseDeterminatorItem
{
    public:
        struct di_attribute {
            std::shared_ptr<boost::regex> re;
            std::string                   value;
        };

        typedef std::map<std::string, struct di_attribute> attribute_t;

    public:
        BaseDeterminatorItem(const std::string& name);
        BaseDeterminatorItem(const pugi::xml_node& basenode, const std::string& name);

        virtual BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const = 0;

        virtual std::string toString() const;
        virtual void parseFromXml(const pugi::xml_node& basenode);
        virtual bool match() const;
        virtual void execute() const;
        std::string getAttribute(const std::string& attrname) const;

        std::string item_name;
        attribute_t attributes;

        //boost::signal2::signal<void ()> sigChanged;
};

template<typename T>
class DeterminatorAction: public BaseDeterminatorItem
{
    public:
        DeterminatorAction() :BaseDeterminatorItem(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0)) { }
        DeterminatorAction(const pugi::xml_node& basenode) 
        :BaseDeterminatorItem(basenode, std::string(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0))) {
            attributes["executeOrder"];
        }
        
        BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const {
            return BaseDeterminatorItemPtr(new T(basenode));
        }
};

template<typename T>
class DeterminatorCondition: public BaseDeterminatorItem
{
    public:
        DeterminatorCondition() :BaseDeterminatorItem(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0)) { }
        DeterminatorCondition(const pugi::xml_node& basenode) 
        :BaseDeterminatorItem(basenode, std::string(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0))) { } 

        BaseDeterminatorItemPtr createNew(const pugi::xml_node& basenode) const {
            return BaseDeterminatorItemPtr(new T(basenode));
        }
};
