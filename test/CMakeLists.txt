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

#project(xPLHALTest)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "-g -Os")
set(xPLHALTest_SRCS test_test.cpp)

add_executable(test_runner ${xPLHALTest_SRCS})
#message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
#message(STATUS "LIBRARIES=${LIBS}")
#target_link_libraries(xPLHAL ${Boost_LIBRARIES} -dynamic xPL pthread -static)
target_link_libraries(test_runner ${LIBS} boost_unit_test_framework-mt)

