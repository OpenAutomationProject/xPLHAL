/*
   xPLHAL implementation in C++
   Copyright (C) 2009 by Christian Mayer - xpl at ChristianMayer dot de

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "xplmessage.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

using namespace boost::algorithm;
using boost::lexical_cast;

std::string xPLMessage::getTypeString() const
{
    switch(type)
    {
        case xPL_MESSAGE_COMMAND: return "xpl-cmnd";
        case xPL_MESSAGE_STATUS:  return "xpl-stat";
        case xPL_MESSAGE_TRIGGER: return "xpl-trig";
        default: return "!UNKNOWN!";
    }
}
        
std::string xPLMessage::getSourceVDI() const
{
    return vendor + std::string("-") + deviceID + std::string(".") + instanceID;
}

std::string xPLMessage::getTargetVDI() const
{
    return targetVendor + std::string("-") + targetDevice + std::string(".") + targetInstance;
}
        
bool xPLMessage::setSourceFromVDI(const std::string& source)
{
    size_t marker1 = source.find( "-" );
    size_t marker2 = source.find( "." );
    vendor     = source.substr( 0, marker1 );
    deviceID   = source.substr( marker1+1, marker2 - (marker1+1) );
    instanceID = source.substr( marker2+1 );

    return true;
}

bool xPLMessage::setTargetFromVDI(const std::string& target)
{
    size_t marker1 = target.find( "-" );
    size_t marker2 = target.find( "." );
    targetVendor   = target.substr( 0, marker1 );
    targetDevice   = target.substr( marker1+1, marker2 - (marker1+1) );
    targetInstance = target.substr( marker2+1 );

    return true;
}
        
std::string xPLMessage::getNamedValue(const std::string& tag) const
{
    for (auto entry : namedValues) {
        if (entry.first == tag) {
            return entry.second;
        }
    }
    return "";
}

std::string xPLMessage::printXPLMessage() const
{
    std::string result;
    result += "<" + getTypeString() + " ";

    /* Source Info */
    result += getSourceVDI();
    result += " -> ";
    /* Handle various target types */
    if (isBroadcastMessage) {
        result += "*";
    } else {
        if (isGroupMessage) {
            result += "XPL-GROUP." + targetGroup;
        } else {
            result += getTargetVDI();
        }
    }

    /* Print hop count */
    result += " (" + lexical_cast<std::string>(hopcount) + " hops) ";

    /* Echo Schema Info */
    result += "[" + msgClass + "." + msgType + "]: ";

    for (auto entry : namedValues) {
        result += entry.first + "=" + entry.second + "\n";
    }
    replace_all(result, "\n", ";");

    result += ">";

    return result;
}

std::ostream& operator<<(std::ostream& os, const xPLMessage& msg)
{
    os << msg.printXPLMessage();
    return os;
}

