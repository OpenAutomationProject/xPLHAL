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


#ifndef IXPLHANDLER_H
#define IXPLHANDLER_H

#include <string>

// this is also including the xPL.h
#include "xplmessagequeue.h"

/**
 * \brief Handle all xPL communication.
 */
class IxPLHandler
{
    public:
        virtual ~IxPLHandler() {}

        /** \brief Broadcast one message to the xPL network. */
        virtual void sendBroadcastMessage( const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const = 0;

        /** \brief Send a directed message to the xPL network. */
        virtual void sendMessage( const xPL_MessageType type, const std::string& tgtVendor, const std::string& tgtDeviceID, 
                const std::string& tgtInstanceID, const std::string& msgClass, const std::string& msgType, 
                const xPLMessage::namedValueList& namedValues ) const = 0;

        /** \brief Send a directed message to the xPL network. */
        virtual void sendMessage( const xPL_MessageType type, const std::string& VDI,
                const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const = 0;

};

#endif // IXPLHANDLER_H
