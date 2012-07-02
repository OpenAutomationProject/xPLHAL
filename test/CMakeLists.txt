#  xPLHAL implementation in C++
#  Copyright (C) 2009 by Christian Mayer - xpl at ChristianMayer dot de
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)

include(CTest)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

file(GLOB_RECURSE UnitTests_sources *.cpp)
file(GLOB_RECURSE Action_sources ../src/actions/*.cpp)
file(GLOB_RECURSE Condition_sources ../src/conditions/*.cpp)

#set(UnitTest_tsrc ../src/devicemanager.cpp ../src/xplcache.cpp ../src/xplhandler.cpp ../src/xplmessagequeue.cpp)
set(UnitTest_tsrc ../src/devicemanager.cpp ../src/xplmessage.cpp ../src/determinator.cpp 
                  ../src/determinatoritems.cpp ../src/pugixml.cpp ../src/determinator_manager.cpp
                  ${Action_sources} ${Condition_sources})

add_executable(test_runner ${UnitTests_sources} ${UnitTest_tsrc})

include_directories(../src /usr/local/include)

enable_testing()

set(CMAKE_CXX_FLAGS "-g -Os -std=c++11 -fprofile-arcs -ftest-coverage")
#set(xPLHALTest_SRCS test_test.cpp test_xplcache.cpp)
#add_executable(test_runner ${xPLHALTest_SRCS})
#message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
#message(STATUS "LIBRARIES=${LIBS}")
#message(STATUS "SOURCE=${SOURCES}")
#target_link_libraries(xPLHAL ${Boost_LIBRARIES} -dynamic xPL pthread -static)
target_link_libraries(test_runner ${LIBS} mockpp)

add_test(UnitTests test_runner)

