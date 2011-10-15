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


#ifndef XPLHANDLER_H
#define XPLHANDLER_H

#include <vector>
#include <string>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

using namespace std;

// this is also including the xPL.h
#include "xplmessagequeue.h"

/**
 * \brief Handle all xPL communication.
 */
class xPLHandler
{
     /** \brief variable to ensure that the xPL library is only called at the same time... */
     //mutable boost::mutex xPLLock;
     //typedef boost::lock_guard<boost::mutex> lock_guard;

  public:
    xPLHandler( const string& host_name );
    ~xPLHandler();

    void run();

    /** \brief Broadcast one message to the xPL network. */
    void sendBroadcastMessage( const string& msgClass, const string& msgType, const xPLMessage::namedValueList& namedValues ) const;

     /** \brief Send a directed message to the xPL network. */
    void sendMessage( const xPL_MessageType type, const string& tgtVendor, const string& tgtDeviceID, 
                      const string& tgtInstanceID, const string& msgClass, const string& msgType, 
                      const xPLMessage::namedValueList& namedValues ) const;

                      /** \brief Send a directed message to the xPL network. */
    void sendMessage( const xPL_MessageType type, const string& VDI,
                      const string& msgClass, const string& msgType, const xPLMessage::namedValueList& namedValues ) const;

  private:
    /** \brief Print via the logging facility the whole content of a xPL message. */
    static void printXPLMessage( xPL_MessagePtr theMessage );

    /** \brief Handle an incomming xPL message. */
    static void handleXPLMessage( xPL_MessagePtr theMessage, xPL_ObjectPtr userValue );

    xPL_ServicePtr xPLService;
    string vendor;
    string deviceID;
    string instanceID;
    boost::thread* m_thread;
};

#endif // XPLHANDLER_H
