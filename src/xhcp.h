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

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "xplcache.h"
#include "xhcpthread.h"


/**
 * \brief Manage all XHCP connetions.
 */
class XHCPServer
{
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::tcp::acceptor acceptor;
  volatile bool m_stoprequested;
  boost::thread m_thread;

  public:
    XHCPServer();
    ~XHCPServer();

  protected:
    /** \brief Create a new XHCPThread for a new connection. */
    void waitForConnection( void );
    void handleAccept(socket_ptr sockPtr);
    void startAccept();
};
