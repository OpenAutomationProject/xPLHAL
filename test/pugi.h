#include <iostream>
#include <pugixml.hpp>
#include <string>
#include <vector>
//#include <boost/shared_ptr.hpp>
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
#include <map>

using std::string;
using std::vector;


class Determinator
{
    public:
        enum class match_type { ALL, ANY };

        string guid;
        string name;
        string description;
        bool enabled;
        match_type input_match_type;
        
        vector<BaseConditionPtr> inputs;
};


void printDeterminator(const Determinator& d)
{
    std::cout << "Determinator '" << d.name << "'";
    std::cout << "\n   guid: " << d.guid;
    std::cout << "\n   name: " << d.name;
    std::cout << "\n   description: " << d.description;
    std::cout << "\n   enabled: " << d.enabled;

    std::cout << "\n   Inputs:\n";
    for (auto input : d.inputs) {
        std::cout << "      " << input->toString() << std::endl;
    }

    std::cout << std::endl;
}

class DeterminatorXmlParser
{
    public:
        DeterminatorXmlParser(const string& filename) 
        {
            registerCondition(BaseConditionConstPtr(new XplCondition));
            registerCondition(BaseConditionConstPtr(new GlobalCondition));
            registerCondition(BaseConditionConstPtr(new GlobalChanged));
            registerCondition(BaseConditionConstPtr(new DayCondition));
            registerCondition(BaseConditionConstPtr(new TimeCondition));

            pugi::xml_parse_result result = m_doc.load_file(filename.c_str());
            std::cout << "Load result: " << result.description() << "\n";
        }

        void registerCondition(BaseConditionConstPtr condition) {
            m_conditionmap[condition->condition_name] = condition;
        }

        void parse() {
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

                    for(auto condition : m_conditionmap) {
                        pugi::xml_node action_node = input.child(condition.first.c_str());
                        if (action_node) {
                            d.inputs.push_back( BaseConditionPtr(condition.second->createNew(action_node)) );
                        }
                    }

                    printDeterminator(d);
                }

            } 
            catch(const std::exception& e) {
                int status;
                char* realname = abi::__cxa_demangle(typeid(e).name(), 0, 0, &status);
                std::cout << "Exception: " << realname << " => " << e.what() << std::endl;
                throw;
            }

        }


    private:
        pugi::xml_node getNode(const pugi::xml_node& base, const string& childname) {
            pugi::xml_node node = base.child(childname.c_str());
            if (!node) {
                throw DeterminatorParseException("node '" + childname +"' not found");
            }
            return node;
        }

        pugi::xml_document m_doc;
        std::map<string, std::shared_ptr<const BaseCondition>> m_conditionmap;
};
   
int main()
{
    DeterminatorXmlParser parser("determinatorDesc.xml");
    parser.parse();

    /*
    try {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("determinatorDesc.xml");
    
    std::cout << "Load result: " << result.description() << "\n";

    pugi::xml_node base = doc.child("xplDeterminator");
    pugi::xml_node base_d = base.child("determinator");

    bool isGroup = base_d.attribute("isGroup").value() == "Y";

    if (isGroup == false) {
        vector<string> actions = {"xplCondition", "globalCondition", "globalChanged", "dayCondition", "timeCondition"};

        Determinator d;
        d.guid = base_d.attribute("guid").value();
        d.name = base_d.attribute("name").value();
        d.description = base_d.attribute("description").value();
        d.enabled = base_d.attribute("guid").value() == "Y";

        pugi::xml_node input = base_d.child("input");

        for(auto action : actions) {
            pugi::xml_node action_node = input.child(action.c_str());
            if (action_node) {
                if (action == "globalCondition") {
                    d.inputs.push_back(DCPtr(new DeterminatorConditionGlobal(action_node)));
                }
                else if (action == "xplCondition") {
                    XplCondition xplcond(action_node);

                }
            }
        }

        printDeterminator(d);
    }

    } 
    catch(const std::exception& e) {
        int status;
        char* realname = abi::__cxa_demangle(typeid(e).name(), 0, 0, &status);
        std::cout << "Exception: " << realname << " => " << e.what() << std::endl;
    }
    */

    
}

/*

 <output>
   <logAction display_name="display-name"
   <xplAction display_name="action-name"
   <globalAction
   <delayAction
   <stopAction
   <suspendAction
   <executeAction
   <execRuleAction
   <runScriptAction
 </output>
*/
