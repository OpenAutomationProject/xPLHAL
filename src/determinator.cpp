#include "determinator.h"
#include <iostream>
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <future>
#include "conditions/xplCondition.h"
#include "conditions/globalCondition.h"
#include "conditions/globalChanged.h"
#include "conditions/dayCondition.h"
#include "conditions/timeCondition.h"
#include "actions/logAction.h"
#include "actions/xplAction.h"
#include "actions/delayAction.h"
#include "actions/globalAction.h"
#include "actions/stopAction.h"

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
    registerCondition(BaseDeterminatorItemConstPtr(new xplCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new globalCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new globalChanged));
    registerCondition(BaseDeterminatorItemConstPtr(new dayCondition));
    registerCondition(BaseDeterminatorItemConstPtr(new timeCondition));

    registerAction(BaseDeterminatorItemConstPtr(new logAction));
    registerAction(BaseDeterminatorItemConstPtr(new xplAction));
    registerAction(BaseDeterminatorItemConstPtr(new delayAction));
    registerAction(BaseDeterminatorItemConstPtr(new globalAction));
    registerAction(BaseDeterminatorItemConstPtr(new stopAction));

    pugi::xml_parse_result result = m_doc.load_file(filename.c_str());
//    cerr << "Load result: " << result.description() << "\n";
}

/**
 * \brief register handler for XML parser
 * this allowes to extend the parser with more conditions
 */
void DeterminatorXmlParser::registerCondition(BaseDeterminatorItemConstPtr condition) 
{
    m_conditionmap[condition->item_name] = condition;
}

/**
 * \brief register handler for XML parser
 * this allowes to extend the parser with more actions
 */
void DeterminatorXmlParser::registerAction(BaseDeterminatorItemConstPtr action)
{
    m_actionmap[action->item_name] = action;
}

/**
 * \brief Parse the filen given in constructor
 * \return Determinator object
 */
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

            //d.printDeterminator();
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
    if (!mExecFutures.empty()) {
        mExecFutures[0].wait();
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

bool Determinator::executeOutputs() const
{
    std::multimap<string, BaseDeterminatorItemPtr> orderd_outputs;
    for (auto output : outputs) {
        string execOrder;
        auto execOrderIter = output.second->attributes.find("executeOrder");
        if (execOrderIter != output.second->attributes.end()) {
            execOrder = execOrderIter->second.value;
        }
        orderd_outputs.insert({execOrder, output.second});
    }

    try {
        for (auto output : orderd_outputs) {
            cerr << "execute output:" << output.second->getAttribute("display_name") << endl;
            output.second->execute();
        }
    } catch (const AbortDeterminatorExecutionException &e) {
        return false;
    }
    return true;
}


/**
 * Check if input conditions are met, then start a thread to execute actions
 */
void Determinator::execute()
{
    if (checkInputs()) {
        if (mExecFutures.empty() || mExecFutures[0].valid()) {
            mExecFutures.clear();
//            mExecFutures.erase(mExecFutures.begin(), mExecFutures.end());
            cerr << "determinator start thread" << endl;
            mExecFutures.emplace_back(std::async(std::launch::async, std::bind(&Determinator::executeOutputs, this)));
            cerr << "determinator start thread" << endl;
        }
    }
}

