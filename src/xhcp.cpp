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

#include "log.h"
#include "xhcpthread.h"

#include "xhcp.h"

XHCPServer::XHCPServer()
 : acceptor(io_service, tcp::endpoint(tcp::v4(), 3865)),
   m_stoprequested(false),  
   m_thread(boost::bind(&XHCPServer::waitForConnection, this))
{}

void XHCPServer::waitForConnection( void )
{
  writeLog( "XHCPServer::waitForConnection", logLevel::debug );
  while (!m_stoprequested)
  {
    std::cerr << "for(;;) start \n";
    socket_ptr sockPtr(new tcp::socket(io_service));
    std::cerr << "for(;;) 1 \n";
    acceptor.accept(*sockPtr);
    std::cerr << "for(;;) 2 \n";
    //boost::thread t(boost::bind(&XHCPServer::session, this, sock));
    std::cerr << "for(;;) use_count: " << sockPtr.use_count() << "\n";
    XHCPThread* foo = new XHCPThread( sockPtr );
    std::cerr << "for(;;) foo: " << int(foo) << "\n";
    std::cerr << "for(;;) end \n";
  }
}
