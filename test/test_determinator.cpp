#include "../src/determinator.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <mockpp/visiting/CountedVisitableMethod.h>

using namespace mockpp;

#include "determinator.h"
#include <boost/regex.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE(DeterminatorSuite);

BOOST_AUTO_TEST_CASE( parse )
{
    DeterminatorXmlParser parser("determinatorAll.xml");
    Determinator d(parser.parse());

    BOOST_CHECK(d.name == "rule-name");
    BOOST_CHECK(d.guid == "rule-guid");
    BOOST_CHECK(d.description == "rule-description");
    BOOST_CHECK(d.enabled == 0);

    {   
        auto c = d.inputs.find("dayCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "dayCondition:\n"
                "display_name: rule-name\n"
                "dow: 0000000");
    }
    {   
        auto c = d.inputs.find("globalChanged");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "globalChanged:\n"
                "display_name: rule-name\n"
                "name: global-name");
    }
    {   
        auto c = d.inputs.find("globalCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "globalCondition:\n"
                "display_name: condition-name\n"
                "name: global-name\n"
                "operator: >=\n"
                "value: global-value");
    }
    {   
        auto c = d.inputs.find("timeCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "timeCondition:\n"
                "display_name: display-name\n"
                "operator: !=\n"
                "value: 19:24");
    }
    {   
        auto c = d.inputs.find("xplCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "xplCondition:\n"
                "display_name: condition-name\n"
                "msg_type: cmnd\n"
                "schema_class: class\n"
                "schema_type: type\n"
                "source_device: device\n"
                "source_instance: instance\n"
                "source_vendor: pnxs\n"
                "target_device: device\n"
                "target_instance: instance\n"
                "target_vendor: vendor\n"
                "parameter: parameter-name<=parameter-value");
    }
    
    {   
        auto c = d.outputs.find("logAction");
        BOOST_CHECK(c != d.outputs.end());
        BOOST_CHECK(c->second->toString() == 
                "logAction:\n"
                "display_name: display-name\n"
                "executeOrder: nnn\n"
                "logText: text to log");
    }
}

BOOST_AUTO_TEST_CASE( parse1 )
{
    DeterminatorXmlParser parser("determinator1.xml");
    Determinator d(parser.parse());
    
    {   
        auto c = d.inputs.find("globalCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "globalCondition:\n"
                "display_name: Gobal variable is 1\n"
                "name: TERRARIUM_ZUSTAND\n"
                "operator: =\n"
                "value: 1");
    }
}

BOOST_AUTO_TEST_CASE( parse2 )
{
    DeterminatorXmlParser parser("determinator2.xml");
    Determinator d(parser.parse());
    
    {   
        auto c = d.inputs.find("xplCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "xplCondition:\n"
                "display_name: Klingel Erna gedrueckt\n"
                "msg_type: trig\n"
                "schema_class: sensor\n"
                "schema_type: basic\n"
                "source_device: ppdev\n"
                "source_instance: default\n"
                "source_vendor: pnxs\n"
                "target_device: *\n"
                "target_instance: *\n"
                "target_vendor: *\n"
                "parameter: device=7\n"
                "parameter: type=input\n"
                "parameter: current=1");
    }
}


BOOST_AUTO_TEST_CASE( execute1 )
{
    DeterminatorXmlParser parser("determinator1.xml");
    Determinator d(parser.parse());

    d.execute();
}

BOOST_AUTO_TEST_SUITE_END();
