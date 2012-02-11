#pragma once
#include "pugixml.hpp"
#include "determinatoritems.h"
#include <string>
#include <vector>
#include <map>

class Determinator
{
    public:
        enum class match_type { ALL, ANY };

        void printDeterminator() const;

        std::string guid;
        std::string name;
        std::string description;
        bool enabled;
        match_type input_match_type;
        
        std::multimap<std::string, BaseDeterminatorItemPtr> inputs;
        std::multimap<std::string, BaseDeterminatorItemPtr> outputs;
};

class DeterminatorXmlParser
{
    public:
        DeterminatorXmlParser(const std::string& filename); 

        void registerCondition(BaseDeterminatorItemConstPtr condition);
        void registerAction(BaseDeterminatorItemConstPtr action);
        Determinator parse();

    private:
        pugi::xml_node getNode(const pugi::xml_node& base, const std::string& childname);

        pugi::xml_document m_doc;
        std::map<std::string, std::shared_ptr<const BaseDeterminatorItem>> m_conditionmap;
        std::map<std::string, std::shared_ptr<const BaseDeterminatorItem>> m_actionmap;
};
   
