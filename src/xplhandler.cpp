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

#include "xplhandler.h"
#include "xplmessagequeue.h"

#include <boost/algorithm/string/replace.hpp>
#include <functional>

using std::string;
using namespace std::placeholders;

#include "log.h"


/** Handle a change to the logger service configuration */
/* static void configChangedHandler(xPL_ServicePtr theService, xPL_ObjectPtr userData) {
} */

int xPLHandler::m_refcount = 0;

xPLHandler::xPLHandler(boost::asio::io_service& ioservice, const std::string& host_name)
: xPLService(0)
, vendor( "CHRISM" )
, deviceID( "xplhalqt" )
, instanceID( host_name )
, m_xplSocket(ioservice)
, m_xplWriteSocket(ioservice)
, mXplMessageQueue(new XplMessageQueue)
{
    writeLog( "xPLHandler::xPLHandler( "+host_name+" )", logLevel::debug );
    //xPL_setDebugging(TRUE);
    /* Start xPL up */
    /*int argc = 0;
      char *argv[] = { "xpl" };
      if ( !xPL_parseCommonArgs( &argc, argv, FALSE ) ) 
      {
      writeLog("Unable to parse xPL", logLevel::debug);
      }*/

    if (m_refcount++ == 0) {
        /* first user, initialize */
        if( !xPL_initialize(xPL_getParsedConnectionType()) ) {
            writeLog("Unable to start xPL", logLevel::debug);
        }
    }
    
    /* And a listener for all xPL messages */
    xPL_addMessageListener( xpl_message_callback, this );

    /* Create service so we can create messages */
    if ( (xPLService = xPL_createService( const_cast<char*>( vendor.c_str()), const_cast<char*>( deviceID.c_str()), const_cast<char*>( instanceID.c_str()) )) == 0 )
    {
        writeLog("Error: Unable to create xPL service", logLevel::error);
    }

    //char A[] = "CHRISM";
    //char B[] = "logger";
    //char C[] = "logger.xpl";
    //xPL_ServicePtr loggerService = xPL_createConfigurableService(A, B, C);
    //xPL_setServiceVersion(loggerService, "1.1");
    /*  if (!xPL_isServiceConfigured(loggerService)) {
        writeLog("!xPL_isServiceConfigured(loggerService)", logLevel::debug);

        xPL_addServiceConfigurable(loggerService, LOG_FILE_CFG_NAME, xPL_CONFIG_RECONF, 1);
        xPL_setServiceConfigValue(loggerService, LOG_FILE_CFG_NAME, "stderr");

        xPL_addServiceConfigurable(loggerService, LOG_APPEND_CFG_NAME, xPL_CONFIG_RECONF, 1);
        xPL_setServiceConfigValue(loggerService, LOG_APPEND_CFG_NAME, "false");
        }

        xPL_addServiceConfigChangedListener(loggerService, configChangedHandler, NULL);*/
    //xPL_setServiceEnabled(loggerService, TRUE);
    xPL_setServiceEnabled(xPLService, TRUE);
    
    m_xplSocket.assign(boost::asio::ip::tcp::v4(), xPL_getFD());
    m_xplWriteSocket.assign(boost::asio::ip::tcp::v4(), mXplMessageQueue->getFD());
    startAsyncRead();
    startAsyncWrite();
}

xPLHandler::~xPLHandler()
{
    writeLog( "xPLHandler::~xPLHandler()", logLevel::debug );
    m_xplSocket.close();
    if (xPLService) {
        xPL_releaseService(xPLService);
    }
    if (--m_refcount == 0) {
        xPL_shutdown();
    }
}

void xPLHandler::sendBroadcastMessage( const string& msgClass, const string& msgType, const xPLMessage::namedValueList& namedValues )
{
    writeLog( "xPLHandler::sendBroadcastMessage( "+msgClass+", "+msgType+" )", logLevel::debug );
    sendMessage( xPLMessagePtr( new xPLMessage( xPL_MESSAGE_COMMAND, "*", "", "", msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const string& tgtVendor, const string& tgtDeviceID, 
        const string& tgtInstanceID, const string& msgClass, const string& msgType, 
        const xPLMessage::namedValueList& namedValues )
{
    writeLog( "xPLHandler::sendMessage( "+lexical_cast<string>(type)+", "+tgtVendor+", "+tgtDeviceID+", "+tgtInstanceID+", "+msgClass+", "+msgType+" )", logLevel::debug );
    sendMessage( xPLMessagePtr( new xPLMessage( type, tgtVendor, tgtDeviceID, tgtInstanceID, msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const string& VDI,
        const string& msgClass, const string& msgType,
        const xPLMessage::namedValueList& namedValues ) 
{
    size_t marker1 = VDI.find( "-" );
    size_t marker2 = VDI.find( "." );
    string vendor   = VDI.substr( 0, marker1 );
    string device   = VDI.substr( marker1+1, marker2 - (marker1+1) );
    string instance = VDI.substr( marker2+1 );
    sendMessage( xPLMessagePtr( new xPLMessage( type, vendor, device, instance, msgClass, msgType, namedValues ) ) );
}

void xPLHandler::xpl_message_callback( xPL_MessagePtr theMessage, void *userValue )
{
    xPLHandler* obj = static_cast<xPLHandler*>(userValue);
    obj->handleXPLMessage(theMessage);
}

void xPLHandler::handleXPLMessage( xPL_MessagePtr theMessage)
{
    xPLMessage::namedValueList values; 
    xPL_NameValueListPtr nvList = xPL_getMessageBody(theMessage);
    xPL_NameValuePairPtr nvPair = NULL;
    int nvIndex = 0;
    int nvCount = xPL_getNamedValueCount(nvList);
    /* Write Name/Value Pairs out */
    for (nvIndex = 0; nvIndex < nvCount; nvIndex++) {
        nvPair = xPL_getNamedValuePairAt(nvList, nvIndex);
    
        if (nvPair->itemValue != NULL) {
            values.push_back( std::make_pair( nvPair->itemName, nvPair->itemValue ) );
        }
    }
    xPLMessagePtr msg( new xPLMessage(    xPL_getMessageType(theMessage),
                                          xPL_getSourceVendor(theMessage), 
                                          xPL_getSourceDeviceID(theMessage),
                                          xPL_getSourceInstanceID(theMessage),
                                          xPL_getSchemaClass(theMessage),
                                          xPL_getSchemaType(theMessage),
                                          values) );

    msg->isBroadcastMessage = xPL_isBroadcastMessage(theMessage);
    msg->isGroupMessage     = xPL_isGroupMessage(theMessage);
    if (!msg->isBroadcastMessage) {
        msg->targetVendor       = xPL_getTargetVendor(theMessage);
        msg->targetDevice       = xPL_getTargetDeviceID(theMessage);
        msg->targetInstance     = xPL_getTargetInstanceID(theMessage);
    }
    if (msg->isGroupMessage) {
        msg->targetGroup        = xPL_getTargetGroup(theMessage);
    }
    
    writeLog(msg->printXPLMessage(), logLevel::debug );

    m_sigRceivedXplMessage(msg);
}
        
void xPLHandler::startAsyncRead()
{
    m_xplSocket.async_read_some(boost::asio::null_buffers(), std::bind(&xPLHandler::handleReadableXplSocket, this, _1));
}

void xPLHandler::startAsyncWrite()
{
    m_xplWriteSocket.async_read_some(boost::asio::null_buffers(), std::bind(&xPLHandler::handleReadableXplMessagequeue, this, _1));
}

void xPLHandler::handleReadableXplMessagequeue(boost::system::error_code ec)
{
    if (!ec) {
        // send waiting messages
        while( xPL_MessagePtr theMessage = mXplMessageQueue->consume( xPLService ) )
        {
            writeLog("Found xPL message at " + lexical_cast<string>(theMessage)+ " to send...", logLevel::debug);
            if ( !xPL_sendMessage( theMessage ) )
                writeLog("Unable to send xPL message", logLevel::debug);
            else
                writeLog( "xPL Message sent...", logLevel::debug );
        }
        startAsyncWrite();
    }
}
        
void xPLHandler::handleReadableXplSocket(boost::system::error_code ec)
{
    if (!ec) {
        // handle messages - and return
        xPL_processMessages(0);
        startAsyncRead();
    }
}

void xPLHandler::sendMessage( const xPLMessagePtr message )
{
    mXplMessageQueue->add(message);
}
