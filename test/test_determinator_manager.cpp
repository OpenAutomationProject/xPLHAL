#include "../src/determinator_manager.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <mockpp/visiting/CountedVisitableMethod.h>

using namespace mockpp;

#include <iostream>

BOOST_AUTO_TEST_SUITE(DeterminatorManagerSuite);

BOOST_AUTO_TEST_CASE( loadDeterminators )
{
    DeterminatorManager dm("determinators");
    dm.loadDeterminators();
}

BOOST_AUTO_TEST_SUITE_END();
