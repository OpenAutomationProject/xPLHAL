#include "../src/determinator.h"
#define BOOST_TEST_MODULE "Determinator"
#define BOOST_TEST_ALTERNATIVE_INIT_API 1
#include <boost/test/unit_test.hpp>
#include <mockpp/visiting/CountedVisitableMethod.h>

using namespace mockpp;

#include "determinator.h"
#include <boost/regex.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE(DeterminatorSuite);

BOOST_AUTO_TEST_CASE( parse )
{
    DeterminatorXmlParser parser("determinatorDesc.xml");
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
                "dow: 0000000");
    }
    {   
        auto c = d.inputs.find("globalChanged");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "globalChanged:\n"
                "name: global-name");
    }
    {   
        auto c = d.inputs.find("globalCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "globalCondition:\n"
                "name: global-name\n"
                "operator: =|!=|>|<|>=|<=\n"
                "value: global-value");
    }
    {   
        auto c = d.inputs.find("timeCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "timeCondition:\n"
                "operator: =|!=|<|>|<=|>=\n"
                "value...: HH:mm");
    }
    {   
        auto c = d.inputs.find("xplCondition");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "xplCondition:\n"
                "msg_type: cmnd|stat|trig|*\n"
                "source_vendor: vendor-name|*\n"
                "source_device: device-name|*\n"
                "source_instance: instance-name|*\n"
                "target_vendor: vendor-name|*\n"
                "target_device: device-name|*\n"
                "target_instance: instance-name|*\n"
                "schema_class: schema-class|*\n"
                "schema_type: schema-type|*\n"
                "parameter: parameter-name=|!=|>|<|>=|<=parameter-value");
    }
    
    {   
        auto c = d.outputs.find("logAction");
        BOOST_CHECK(c != d.inputs.end());
        BOOST_CHECK(c->second->toString() == 
                "logAction:\n"
                "logText.....: text to log\n"
                "executeOrder: nnn");
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
                "msg_type: trig\n"
                "source_vendor: pnxs\n"
                "source_device: ppdev\n"
                "source_instance: default\n"
                "target_vendor: *\n"
                "target_device: *\n"
                "target_instance: *\n"
                "schema_class: sensor\n"
                "schema_type: basic\n"
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
