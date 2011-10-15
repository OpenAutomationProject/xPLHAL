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
#include "devicemanager.h"

#include "xplhandler.h"

/** Handle a change to the logger service configuration */
/* static void configChangedHandler(xPL_ServicePtr theService, xPL_ObjectPtr userData) {
} */

extern bool xPLHandlerInitialized;

xPLHandler::xPLHandler( const string& host_name )
     : xPLService(0),
     vendor( "CHRISM" ), deviceID( "xplhalqt" ), instanceID( host_name )
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
  
  if( !xPL_initialize( xPL_getParsedConnectionType() ) )
  {
    writeLog("Unable to start xPL", logLevel::debug);
  }
 
  /* And a listener for all xPL messages */
  xPL_addMessageListener( handleXPLMessage, NULL );

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
//  xPLHandlerInitialized = true;
}

xPLHandler::~xPLHandler()
{
  writeLog( "xPLHandler::~xPLHandler()", logLevel::debug );
  m_thread->join();
  delete m_thread;
}


void xPLHandler::run()
{
  writeLog( "xPLHandler::run()", logLevel::debug );
//  while( !xPLHandlerInitialized ) ; // FIXME make synchronisation a bit more "stable"
//  writeLog( "xPLHandler::run() - ready", logLevel::debug );

  // Hand control over to xPLLib 
  while( true )
  {
    bool atLeastOneMessageProcessed;
    {
      // get exclusive access to xPLib
      //lock_guard locker( xPLLock );

      // send waiting messages
      while( xPL_MessagePtr theMessage = xPLMessageQueue->consume( xPLService ) )
      {
        writeLog("Found xPL message at " + lexical_cast<string>(theMessage)+ " to send...", logLevel::debug);
        if ( !xPL_sendMessage( theMessage ) )
          writeLog("Unable to send xPL message", logLevel::debug);
        else
          writeLog( "xPL Message sent...", logLevel::debug );
      }

      // handle messages - and return immediately
      atLeastOneMessageProcessed = xPL_processMessages(0);
    }
    if( !atLeastOneMessageProcessed )
      sleep( 1 );                     // wait a bit for the next iteration
  }
}

void xPLHandler::sendBroadcastMessage( const string& msgClass, const string& msgType, const xPLMessage::namedValueList& namedValues ) const
{
  writeLog( "xPLHandler::sendBroadcastMessage( "+msgClass+", "+msgType+" )", logLevel::debug );
  xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( xPL_MESSAGE_COMMAND, "*", "", "", msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const string& tgtVendor, const string& tgtDeviceID, 
                              const string& tgtInstanceID, const string& msgClass, const string& msgType, 
                              const xPLMessage::namedValueList& namedValues ) const
{
  writeLog( "xPLHandler::sendMessage( "+lexical_cast<string>(type)+", "+tgtVendor+", "+tgtDeviceID+", "+tgtInstanceID+", "+msgClass+", "+msgType+" )", logLevel::debug );
  xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( type, tgtVendor, tgtDeviceID, tgtInstanceID, msgClass, msgType, namedValues ) ) );
}

void xPLHandler::sendMessage( const xPL_MessageType type, const string& VDI,
                              const string& msgClass, const string& msgType,
                              const xPLMessage::namedValueList& namedValues ) const
{
  size_t marker1 = VDI.find( "-" );
  size_t marker2 = VDI.find( "." );
  string vendor   = VDI.substr( 0, marker1 );
  string device   = VDI.substr( marker1+1, marker2 - (marker1+1) );
  string instance = VDI.substr( marker2+1 );
  xPLMessageQueue->add( xPLMessagePtr( new xPLMessage( type, vendor, device, instance, msgClass, msgType, namedValues ) ) );
}

void xPLHandler::printXPLMessage( xPL_MessagePtr theMessage )
{
  string result;

  /* Source Info */
  result += xPL_getSourceVendor(theMessage);
  result += "-";
  result += xPL_getSourceDeviceID(theMessage);
  result +=  ".",
  result += xPL_getSourceInstanceID(theMessage);

  result += " -> ";
  /* Handle various target types */
  if (xPL_isBroadcastMessage(theMessage)) {
    result += "*";
  } else {
    if (xPL_isGroupMessage(theMessage)) {
      result += "XPL-GROUP.";
      result += xPL_getTargetGroup(theMessage);
    } else {
      result += xPL_getTargetVendor(theMessage);
      result += "-";
      result += xPL_getTargetDeviceID(theMessage);
      result += ".";
      result += xPL_getTargetInstanceID(theMessage);
    }
  }

  /* Print hop count */
  result += " (";
  result += lexical_cast<string>(xPL_getHopCount(theMessage));
  result += " hops) ";

  result += "[";
  switch(xPL_getMessageType(theMessage)) 
  {
    case xPL_MESSAGE_COMMAND:
      result += "xpl-cmnd";
      break;
    case xPL_MESSAGE_STATUS:
      result += "xpl-stat";
      break;
    case xPL_MESSAGE_TRIGGER:
      result += "xpl-trig";
      break;
    default:
      result += "!UNKNOWN!";
      break;
  }
  result += "] ";

  /* Echo Schema Info */
  result += xPL_getSchemaClass(theMessage);
  result += ".";
  result += xPL_getSchemaType(theMessage);
  result += ": ";

  xPL_NameValueListPtr nvList = xPL_getMessageBody(theMessage);
  xPL_NameValuePairPtr nvPair = NULL;
  int nvIndex = 0;
  int nvCount = xPL_getNamedValueCount(nvList);
  /* Write Name/Value Pairs out */
  for (nvIndex = 0; nvIndex < nvCount; nvIndex++) {
    nvPair = xPL_getNamedValuePairAt(nvList, nvIndex);
    result += nvPair->itemName; //WRITE_TEXT(nvPair->itemName);
    result += "="; //WRITE_TEXT("=");

    /* Write data content out */
    if (nvPair->itemValue != NULL) {
      if (nvPair->isBinary) 
        ;//writeBinaryValue(nvPair->itemValue, nvPair->binaryLength);
      else
        result += nvPair->itemValue; //WRITE_TEXT(nvPair->itemValue);
    }

    /* Terminate line/entry */
    result += "\n"; //WRITE_TEXT("\n");
  }
  replace_all( result, "\n", ";" );
  
  writeLog( result, logLevel::debug );
}

void xPLHandler::handleXPLMessage( xPL_MessagePtr theMessage, xPL_ObjectPtr UNUSED userValue )
{
  printXPLMessage( theMessage );

  //RaiseEvent ParseMessageForCache(e.XplMsg)
  //RaiseEvent ParseMessageForRules(e.XplMsg)

  string schema = xPL_getSchemaClass(theMessage) + string(".") + xPL_getSchemaType(theMessage);
  if( xPL_getMessageType(theMessage) != xPL_MESSAGE_COMMAND )
  {
    if(        "config.list"    == schema )
    {
      // someone (probably we) have asked for the config list - handle it now...
      deviceManager->processConfigList( theMessage );
    } else if( "config.current" == schema )
    {
      // someone requested the device to send it's current configuration
      deviceManager->processCurrentConfig( theMessage );
    } else if( "config.app"     == schema || "config.basic" == schema )
    {
      // a new device poped up and wants to be configured
      deviceManager->processConfigHeartBeat( theMessage );
    } else if( "hbeat.basic"    == schema || "hbeat.app"    == schema )
    {
      /*
      If msgSource = MySourceTag Then
        RaiseEvent AddtoCache("xplhal." & msgSource & ".alive", Now.ToString, False)
      End If
      */
      deviceManager->processHeartbeat( theMessage );
    } else if( "hbeat.end"      == schema )
    {
      deviceManager->processRemove( theMessage );
    }
  }
}