#include "determinator.h"
#include <iostream>
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
#include <functional>

using std::string;
using std::vector;
using std::cerr;
using std::endl;
using std::thread;
using std::bind;

void Determinator::printDeterminator() const
{
    cerr << "Determinator '" << name << "'";
    cerr << "\n   guid: " << guid;
    cerr << "\n   name: " << name;
    cerr << "\n   description: " << description;
    cerr << "\n   enabled: " << enabled;

    cerr << "\n   Inputs:\n";
    for (auto input : inputs) {
        cerr << "      " << input.second->toString() << endl;
    }
    
    cerr << "\n   Outputs:\n";
    for (auto output : outputs) {
        cerr << "      " << output.second->toString() << endl;
    }

    cerr << endl;
}

DeterminatorXmlParser::DeterminatorXmlParser(const string& filename) 
{
    registerCondition(BaseDeterminatorItemConstPtr(new XplCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new GlobalCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new GlobalChanged));
    registerCondition(BaseDeterminatorItemConstPtr(new DayCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new TimeCondition));

    registerAction(BaseDeterminatorItemConstPtr(new logAction));
    registerAction(BaseDeterminatorItemConstPtr(new xplAction));

    pugi::xml_parse_result result = m_doc.load_file(filename.c_str());
    cerr << "Load result: " << result.description() << "\n";
}

void DeterminatorXmlParser::registerCondition(BaseDeterminatorItemConstPtr condition) 
{
    m_conditionmap[condition->item_name] = condition;
}

void DeterminatorXmlParser::registerAction(BaseDeterminatorItemConstPtr action)
{
    m_actionmap[action->item_name] = action;
}

Determinator DeterminatorXmlParser::parse() 
{
    try {
        pugi::xml_node base = getNode(m_doc, "xplDeterminator");
        pugi::xml_node base_d = getNode(base, "determinator");

        bool isGroup = base_d.attribute("isGroup").value() == "Y";

        if (isGroup == false) {
            Determinator d;
            d.guid = base_d.attribute("guid").value();
            d.name = base_d.attribute("name").value();
            d.description = base_d.attribute("description").value();
            d.enabled = base_d.attribute("guid").value() == "Y";

            pugi::xml_node input = base_d.child("input");
            d.input_match_type = Determinator::match_type::ALL;
            if (input.attribute("match").value() == "any") {
                d.input_match_type = Determinator::match_type::ANY;
            }

            pugi::xml_node output = base_d.child("output");

            for(auto condition : m_conditionmap) {
                pugi::xml_node action_node = input.child(condition.first.c_str());
                if (action_node) {
                    d.inputs.insert({condition.first, BaseDeterminatorItemPtr(condition.second->createNew(action_node))} );
                }
            }
            for(auto action : m_actionmap) {
                pugi::xml_node action_node = output.child(action.first.c_str());
                if (action_node) {
                    BaseDeterminatorItemPtr actionObject(action.second->createNew(action_node));
                    d.outputs.insert({action.first, actionObject});
                }
            }

            d.printDeterminator();
            return d;
        }

    } 
    catch(const std::exception& e) {
        int status;
        char* realname = abi::__cxa_demangle(typeid(e).name(), 0, 0, &status);
        cerr << "Exception: " << realname << " => " << e.what() << endl;
        throw;
    }

}


pugi::xml_node DeterminatorXmlParser::getNode(const pugi::xml_node& base, const string& childname) 
{
    pugi::xml_node node = base.child(childname.c_str());
    if (!node) {
        throw DeterminatorParseException("node '" + childname +"' not found");
    }
    return node;
}
        
Determinator::Determinator()
{
}

Determinator::~Determinator()
{
    if (mExecuteThread) {
        mExecuteThread->join();
    }
}
        
bool Determinator::checkInputs() const
{
    for (auto input : inputs) {
        bool match = input.second->match();
        switch(input_match_type) {
            case match_type::ALL: if (!match) return false;
            case match_type::ANY: if (match) return true;
        }
    }
    return (input_match_type == match_type::ALL);
}

void Determinator::executeOutputs() const
{
    std::multimap<string, BaseDeterminatorItemPtr> orderd_outputs;
    for (auto output : outputs) {
        string execOrder;
        auto execOrderIter = output.second->attributes.find("executeOrder");
        if (execOrderIter != output.second->attributes.end()) {
            execOrder = execOrderIter->second;
        }
        orderd_outputs.insert({execOrder, output.second});
    }

    for (auto output : orderd_outputs) {
        cerr << "execute output:" << output.second->display_name << endl;
        output.second->execute();
    }
}

/**
 * Check if input conditions are met, then start a thread to execute actions
 */
void Determinator::execute()
{
    if (checkInputs()) {
        if (mExecuteThread) {
            mExecuteThread->join();
        }
        cerr << "determinator start thread" << endl;
        mExecuteThread.reset(new thread(bind(&Determinator::executeOutputs, this)));
    }
}

