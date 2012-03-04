#include "../src/determinator.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <mockpp/visiting/CountedVisitableMethod.h>
#include "../src/conditions/dayCondition.h"
#include "../src/conditions/timeCondition.h"

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

BOOST_AUTO_TEST_CASE( testdayConditon )
{
    dayCondition dc1({{"dow", {0, "1111111"}}});
    BOOST_CHECK(dc1.match() == true);

    dayCondition dc2({{"dow", {0, "0000000"}}});
    BOOST_CHECK(dc2.match() == false);
}

BOOST_AUTO_TEST_CASE( testtimeCondition )
{
    // 1330895302 => So 4. Mär 22:08:23 CET 2012
    {
        // Equal
        timeCondition tcEq1({{"operator", {0, "="}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcEq1.match() == true);
        timeCondition tcEq2({{"operator", {0, "="}}, {"value", {0, "22:09"}}}, 1330895302);
        BOOST_CHECK(tcEq2.match() == false);
    }
    
    {
        // Not Equal
        timeCondition tcNEq1({{"operator", {0, "!="}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcNEq1.match() == false);
        timeCondition tcNEq2({{"operator", {0, "!="}}, {"value", {0, "22:09"}}}, 1330895302);
        BOOST_CHECK(tcNEq2.match() == true);
    }
    
    {
        // Greater
        timeCondition tcGt1({{"operator", {0, ">"}}, {"value", {0, "20:00"}}}, 1330895302);
        BOOST_CHECK(tcGt1.match() == true);
        timeCondition tcGt2({{"operator", {0, ">"}}, {"value", {0, "23:00"}}}, 1330895302);
        BOOST_CHECK(tcGt2.match() == false);
        timeCondition tcGt3({{"operator", {0, ">"}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcGt3.match() == false);
    }
    
    {
        // Less
        timeCondition tcLt1({{"operator", {0, "<"}}, {"value", {0, "20:00"}}}, 1330895302);
        BOOST_CHECK(tcLt1.match() == false);
        timeCondition tcLt2({{"operator", {0, "<"}}, {"value", {0, "23:00"}}}, 1330895302);
        BOOST_CHECK(tcLt2.match() == true);
        timeCondition tcLt3({{"operator", {0, "<"}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcLt3.match() == false);
    }

    {
        // Greater or Equal
        timeCondition tcGoE1({{"operator", {0, ">="}}, {"value", {0, "20:00"}}}, 1330895302);
        BOOST_CHECK(tcGoE1.match() == true);
        timeCondition tcGoE2({{"operator", {0, ">="}}, {"value", {0, "23:00"}}}, 1330895302);
        BOOST_CHECK(tcGoE2.match() == false);
        timeCondition tcGoE3({{"operator", {0, ">="}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcGoE3.match() == true);
    }
    
    {
        // Less or Equal
        timeCondition tcLoE1({{"operator", {0, "<="}}, {"value", {0, "20:00"}}}, 1330895302);
        BOOST_CHECK(tcLoE1.match() == false);
        timeCondition tcLoE2({{"operator", {0, "<="}}, {"value", {0, "23:00"}}}, 1330895302);
        BOOST_CHECK(tcLoE2.match() == true);
        timeCondition tcLoE3({{"operator", {0, "<="}}, {"value", {0, "22:08"}}}, 1330895302);
        BOOST_CHECK(tcLoE3.match() == true);
    }
}

BOOST_AUTO_TEST_SUITE_END();
