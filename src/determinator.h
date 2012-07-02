#pragma once
#include "pugixml.hpp"
#include "determinatoritems.h"
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <future>

/**
 * \brief Determinator implementation.
 *
 * A Determinator is configured with an XML-File. 
 * When all input conditions are met (true) the Determinator is executed:
 * All output actions are executed in the 'executeOrder' order.
 */
class Determinator
{
    public:
        explicit Determinator();
        virtual ~Determinator();

        enum class match_type { ALL, ANY };

        /**
         * \brief Print Determinator in human-readable form to console
         */
        void printDeterminator() const;

        /**
         * \brief Check if input conditions are met, then start a thread to execute actions
         * The output-actions are executed in a seperate thread.
         */
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
        bool executeOutputs() const;

        std::vector<std::shared_future<bool>> mExecFutures;
};

typedef std::shared_ptr<Determinator> DeterminatorPtr;
typedef std::shared_ptr<const Determinator> DeterminatorConstPtr;

/**
 * \brief Converts a Determinator from XML to internal Object format
 */
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
   
