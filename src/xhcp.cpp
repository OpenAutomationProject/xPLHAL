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

using boost::asio::ip::tcp;

XHCPServer::XHCPServer()
 : acceptor(io_service, tcp::endpoint(tcp::v4(), 3865)),
   m_stoprequested(false),
   m_thread(boost::bind(&XHCPServer::waitForConnection, this))
{
    startAccept();
}

XHCPServer::~XHCPServer()    
{
    m_stoprequested = true;
    acceptor.cancel();
    acceptor.close();
    io_service.stop();
    m_thread.join();
}

void XHCPServer::startAccept()
{
    socket_ptr sockPtr(new tcp::socket(io_service));
    acceptor.async_accept(*sockPtr, boost::bind(&XHCPServer::handleAccept, this, sockPtr));
}

void XHCPServer::handleAccept(socket_ptr sockPtr)
{
    startAccept();
    XHCPThread* foo = new XHCPThread( sockPtr );
}

void XHCPServer::waitForConnection( void )
{
    writeLog( "XHCPServer::waitForConnection", logLevel::debug );
    io_service.run();
    /*
    while (!m_stoprequested)
    {
        socket_ptr sockPtr(new tcp::socket(io_service));
        writeLog("XHCPServer::waitForConnection in accept", logLevel::debug);
        acceptor.accept(*sockPtr);
        writeLog("XHCPServer::waitForConnection left accept", logLevel::debug);

        XHCPThread* foo = new XHCPThread( sockPtr );
    }
    */
}
