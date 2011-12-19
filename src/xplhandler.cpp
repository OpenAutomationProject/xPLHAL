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

#include <boost/algorithm/string/replace.hpp>

using namespace boost::algorithm;

#include "log.h"

#include "xplhandler.h"
#include "globals.h"

/** Handle a change to the logger service configuration */
/* static void configChangedHandler(xPL_ServicePtr theService, xPL_ObjectPtr userData) {
} */

int xPLHandler::m_refcount = 0;

xPLHandler::xPLHandler( const std::string& host_name)
: xPLService(0)
, m_exit_thread(false)
, vendor( "CHRISM" )
, deviceID( "xplhalqt" )
, instanceID( host_name )
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

    m_thread = new boost::thread(boost::bind(&xPLHandler::run, this));
}

xPLHandler::~xPLHandler()
{
    writeLog( "xPLHandler::~xPLHandler()", logLevel::debug );
    m_exit_thread = true;
    m_thread->join();
    delete m_thread;
    if (xPLService) {
        xPL_releaseService(xPLService);
    }
    if (--m_refcount == 0) {
        xPL_shutdown();
    }
}

void xPLHandler::run()
{
    writeLog( "xPLHandler::run()", logLevel::debug );
    //  writeLog( "xPLHandler::run() - ready", logLevel::debug );

    // Hand control over to xPLLib 
    while( ! m_exit_thread )
    {
        // get exclusive access to xPLib
        //lock_guard locker( xPLLock );

        // send waiting messages
        while( xPL_MessagePtr theMessage = xPLMessageQueue->consume( xPLService ) )
        {
            writeLog("Found xPL message at " + lexical_cast<std::string>(theMessage)+ " to send...", logLevel::debug);
            if ( !xPL_sendMessage( theMessage ) )
                writeLog("Unable to send xPL message", logLevel::debug);
            else
                writeLog( "xPL Message sent...", logLevel::debug );
        }

        // handle messages - and return after 100 ms
        xPL_processMessages(100);
    }
}

void xPLHandler::sendBroadcastMessage( const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const
{
    writeLog( "xPLHandler::sendBroadcastMessage( "+msgClass+", "+msgType+" )", logLevel::debug );
    xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( xPL_MESSAGE_COMMAND, "*", "", "", msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const std::string& tgtVendor, const std::string& tgtDeviceID, 
        const std::string& tgtInstanceID, const std::string& msgClass, const std::string& msgType, 
        const xPLMessage::namedValueList& namedValues ) const
{
    writeLog( "xPLHandler::sendMessage( "+lexical_cast<std::string>(type)+", "+tgtVendor+", "+tgtDeviceID+", "+tgtInstanceID+", "+msgClass+", "+msgType+" )", logLevel::debug );
    xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( type, tgtVendor, tgtDeviceID, tgtInstanceID, msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const std::string& VDI,
        const std::string& msgClass, const std::string& msgType,
        const xPLMessage::namedValueList& namedValues ) const
{
    size_t marker1 = VDI.find( "-" );
    size_t marker2 = VDI.find( "." );
    std::string vendor   = VDI.substr( 0, marker1 );
    std::string device   = VDI.substr( marker1+1, marker2 - (marker1+1) );
    std::string instance = VDI.substr( marker2+1 );
    xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( type, vendor, device, instance, msgClass, msgType, namedValues ) ) );
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
