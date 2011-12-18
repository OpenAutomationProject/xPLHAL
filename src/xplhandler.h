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

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/signals2/signal.hpp>

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
        typedef boost::signals2::signal<void (xPLMessagePtr)> signal_t;

    public:
        xPLHandler( const std::string& host_name);
        ~xPLHandler();

        boost::signals2::connection connect(const signal_t::slot_type &subscriber);

        void run();

        /** \brief Broadcast one message to the xPL network. */
        void sendBroadcastMessage( const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const;

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& tgtVendor, const std::string& tgtDeviceID, 
                const std::string& tgtInstanceID, const std::string& msgClass, const std::string& msgType, 
                const xPLMessage::namedValueList& namedValues ) const;

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& VDI,
                const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const;

    private:
        /** \brief Print via the logging facility the whole content of a xPL message. */
        static void printXPLMessage( xPL_MessagePtr theMessage );

        /** \brief Handle an incomming xPL message. */
        void handleXPLMessage( xPL_MessagePtr theMessage);

        /** \brief Handle an incomming xPL message. */
        static void xpl_message_callback( xPL_MessagePtr theMessage, xPL_ObjectPtr userValue );

        xPL_ServicePtr xPLService;
        std::string vendor;
        std::string deviceID;
        std::string instanceID;
        boost::thread* m_thread;
        static int m_refcount;
        bool m_exit_thread;
        signal_t m_sigRceivedXplMessage;
};
