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

#ifndef XHCP_H
#define XHCP_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "xplcache.h"

using boost::asio::ip::tcp;

/**
 * \brief Manage all XHCP connetions.
 */
class XHCPServer
{
  boost::asio::io_service io_service;
  tcp::acceptor acceptor;
  volatile bool m_stoprequested;
  boost::thread m_thread;

  public:
    XHCPServer();
    ~XHCPServer()
    {
        m_stoprequested = true;
        m_thread.join();
    }

  protected:
    /** \brief Create a new XHCPThread for a new connection. */
    void waitForConnection( void );
};

#endif // XHCP_H
