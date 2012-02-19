#pragma once
#include "pugixml.hpp"
#include "determinatoritems.h"
#include <string>
#include <vector>
#include <map>
#include <thread>

class Determinator
{
    public:
        explicit Determinator();
        virtual ~Determinator();

        enum class match_type { ALL, ANY };

        void printDeterminator() const;

        void execute();

        std::string guid;
        std::string name;
        std::string description;
        bool enabled;
        match_type input_match_type;
        
        std::multimap<std::string, BaseDeterminatorItemPtr> inputs;
        std::multimap<std::string, BaseDeterminatorItemPtr> outputs;

    private:
        bool checkInputs() const;
        void executeOutputs() const;
        
        std::unique_ptr<std::thread> mExecuteThread;
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
   
