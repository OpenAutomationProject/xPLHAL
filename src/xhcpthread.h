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

#ifndef XHCPTHREAD_H
#define XHCPTHREAD_H

#include <map>

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>


#include "xplcache.h"

typedef boost::shared_ptr<boost::asio::ip::tcp::tcp::socket> socket_ptr;

/**
 * \brief Single connection to an xPLHAL Manager.
 *
 * Each XHCP connection of an xPLHAL Manager will be handled by it's own 
 * thread. So it's possible to use blocking networking operations and still
 * leave the rest of the xPLHAL responsive.
 */
class XHCPThread
{
    volatile bool m_stoprequested;
    const std::string tab;
    const std::string newLine;
    const std::string endMultiLine;

  public:
    XHCPThread( socket_ptr socket );
    ~XHCPThread();

    /** \brief The main loop of the thread. */
    void run();

  private:
    /** \brief Pointer to an method that takes care of an XHCP command. */
    typedef std::string (XHCPThread::*XHCPcommand)(const std::string&);
    /** \brief Store of all known XHCP command */
    std::map<std::string, XHCPcommand > commands;

    /** \brief The socket belonging to the current connetion. */
    int socketDescriptor;
    socket_ptr sock;
    /** \brief True if the thread should close at the next posibility. */
    bool quit;

    /** \brief True if inside a request where the sender uses multiple lines. */
    bool inMultilineRequest;
    /** \brief The method to call after recieving additional lines for the current request. */
    XHCPcommand multilineRequestHandler;

    std::string commandNotRecognised    ( const std::string& parameter );
    std::string commandCapabilities     ( const std::string& parameter );
    std::string commandDelGlobal        ( const std::string& parameter );
    std::string commandDelDevConfig     ( const std::string& parameter );
    std::string commandGetDevConfig     ( const std::string& parameter );
    std::string commandGetDevConfigValue( const std::string& parameter );
    std::string commandListAllDevices   ( const std::string& parameter );
    std::string commandListDevices      ( const std::string& parameter );
    std::string commandListGlobals      ( const std::string& parameter );
    std::string commandListOptions      ( const std::string& parameter );
    std::string commandListRuleGroups   ( const std::string& parameter );
    std::string commandListRules        ( const std::string& parameter );
    std::string commandPutDevConfig     ( const std::string& parameter );
    std::string commandSetGlobal        ( const std::string& parameter );
    std::string commandSendXPlMessage   ( const std::string& parameter );
    std::string commandQuit             ( const std::string& parameter );

    boost::thread m_thread;
};

#endif // XHCPTHREAD_H
