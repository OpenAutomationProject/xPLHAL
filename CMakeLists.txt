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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
#set(COMPILE_FLAGS "-Wall -Wextra -pedantic")
#set(CMAKE_CXX_COMPILER lsbc++)
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(create_STATIC_build OFF)

if(create_STATIC_build)
  message("Creating a static build...")
  set(Boost_USE_STATIC_LIBS   ON)
  #set(CMAKE_EXE_LINKER_FLAGS  -static)
else(create_STATIC_build)
  message("Creating a dynamic build...")
  set(Boost_USE_STATIC_LIBS   OFF)
endif(create_STATIC_build)

#set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.38 COMPONENTS thread system filesystem date_time regex unit_test_framework program_options)

find_package(xPL REQUIRED)
set(LIBS ${LIBS} ${Boost_LIBRARIES} ${xPL_LIBRARIES} pthread)


#link_directories ( ${Boost_LIBRARY_DIRS} /usr/local/lib/ )
link_directories ( ${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS} ${xPL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} )

#message(STATUS "Top: Boost_LIBRARIES=${Boost_LIBRARIES}")
#message(STATUS "Top: Boost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}")

message( "Building type: ${CMAKE_BUILD_TYPE}" )
if(Boost_USE_STATIC_LIBS)
  message("Linking Boost statically...")
  #message(STATUS "Prior: Boost_LIBRARIES=${Boost_LIBRARIES}")
  #message(STATUS "Prior: LINK_SEARCH_END_STATIC=${LINK_SEARCH_END_STATIC}")
  set( LINK_SEARCH_END_STATIC ON )
  # force inclusion of pthread...
  set(Boost_LIBRARIES ${Boost_LIBRARIES} pthread)
  #message(STATUS "Post: Boost_LIBRARIES=${Boost_LIBRARIES}")
  #message(STATUS "Post: LINK_SEARCH_END_STATIC=${LINK_SEARCH_END_STATIC}")
else(Boost_USE_STATIC_LIBS)
  message("Linking Boost dynamically...")
endif(Boost_USE_STATIC_LIBS)

add_subdirectory(src)
add_subdirectory(test)

