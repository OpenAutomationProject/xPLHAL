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

project(xPLHAL)
cmake_minimum_required(VERSION 2.6)

file(GLOB_RECURSE Action_sources ./actions/*.cpp)
file(GLOB_RECURSE Condition_sources ./conditions/*.cpp)

#set(CMAKE_CXX_FLAGS "-g -std=c++0x")
set(CMAKE_CXX_FLAGS "-g -O2 -std=c++0x")
set(xPLHAL_SRCS xplmessagequeue.cpp devicemanager.cpp 
    xplhandler.cpp xplcache.cpp xhcpthread.cpp log.cpp 
    xhcp.cpp xplmessage.cpp recurring_timer.cpp main.cpp pugixml.cpp
    determinatoritems.cpp determinator.cpp determinator_manager.cpp
    ${Action_sources} ${Condition_sources})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(xPLHAL ${xPLHAL_SRCS})
#message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
#message(STATUS "LIBRARIES=${LIBS}")
#target_link_libraries(xPLHAL ${Boost_LIBRARIES} -dynamic xPL pthread -static)
target_link_libraries(xPLHAL ${LIBS})

