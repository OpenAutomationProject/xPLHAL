#pragma once
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

#include <vector>
#include <string>
#include <memory>

extern "C" {
#define COMMON_TYPES
typedef bool Bool;
typedef const char * String;
#include "xPL.h"
}

/**
 * \brief The C++ version of an xPL message
 */
class xPLMessage
{
    public:
        /** \brief Type that contains all name-value pairs of one xPL message. 
         *  This type has to preserve the order of it's items and must support
         *  multiple entries with the same key. So a map or multimap wan't sufficient.
         */
        typedef std::vector<std::pair<std::string,std::string> > namedValueList;

    public:
        xPL_MessageType type;
        std::string vendor;
        std::string deviceID;
        std::string instanceID;
        std::string msgClass;
        std::string msgType;

        namedValueList namedValues;

        bool isBroadcastMessage;
        bool isGroupMessage;

        std::string targetVendor;
        std::string targetDevice;
        std::string targetInstance;
        std::string targetGroup;

        int hopcount;
        
        xPLMessage() : hopcount(0), isBroadcastMessage(true), isGroupMessage(false)
        {}

        xPLMessage(
                const xPL_MessageType _type,
                const std::string& _vendor,
                const std::string& _deviceID,
                const std::string& _instanceID,
                const std::string& _msgClass,
                const std::string& _msgType,
                const namedValueList& _namedValues
                ) : type(_type), vendor(_vendor), deviceID(_deviceID), instanceID(_instanceID),
                    msgClass(_msgClass), msgType(_msgType), namedValues(_namedValues),
                    hopcount(0), isBroadcastMessage(true), isGroupMessage(false)
        {
        }

        xPLMessage(
                const xPL_MessageType _type,
                const std::string& _vdi,
                const std::string& _msgClass,
                const std::string& _msgType,
                const namedValueList& _namedValues
                ) : type(_type), msgClass(_msgClass), msgType(_msgType), namedValues(_namedValues), 
                    hopcount(0), isBroadcastMessage(true), isGroupMessage(false)
        {
            setSourceFromVDI(_vdi);
        }

        std::string printXPLMessage() const;
        std::string getTypeString() const;
        std::string getSourceVDI() const;
        std::string getTargetVDI() const;

        bool setSourceFromVDI(const std::string& vdi);
        bool setTargetFromVDI(const std::string& vdi);
        
        std::string getNamedValue(const std::string& tag) const;

        operator std::string() const;
        bool operator==(const xPLMessage& right) const;
};
typedef std::shared_ptr<xPLMessage> xPLMessagePtr;

std::ostream& operator<<(std::ostream& os, const xPLMessage& msg);
